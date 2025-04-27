#!/usr/bin/python3

###
### The build system
###

import os
import sys
import subprocess
from typing import Callable

ESC_AQUA = '\x1b[96m' #]
ESC_RST = '\x1b[0m' #]
ESC_ERR = '\x1b[1;91m' #]

BUILD_DEBUG = False

os.chdir(os.path.dirname(__file__))
steps = {}

def step(
		out : str,
		cmd : Callable | list[str] | list[list[str] | Callable],
		deps : list[str] = [],
		phony : bool = False
		):
	if out in steps:
		print(f'{ESC_ERR}Error: duplicate step {out}{ESC_RST}')
		exit(1)

	if not isinstance(cmd, list) or isinstance(cmd[0], str):
		cmd = [cmd]

	steps[out] = {
			'cmd': cmd,
			'deps': deps,
			'phony': phony
	}

def refresh(target : str):
	if target not in steps and not os.path.exists(target):
		print(f'{ESC_ERR}Error: missing rule to build `{target}`!{ESC_RST}')
		exit(1)

	target_time = os.path.getmtime(target) if os.path.exists(target) else 0

	if target not in steps:
		return target_time

	step = steps[target]

	dep_time = target_time+1 if step['phony'] else target_time-1
	outdated = '<PHONY TARGET>'

	for dep in step['deps']:
		dtime = refresh(dep)
		if dtime > dep_time:
			dep_time = dtime
			outdated = dep

	if dep_time < target_time:
		if BUILD_DEBUG:
			print(f'{ESC_AQUA}{target} ({target_time}) is up-to-date, newset dep is {outdated} ({dep_time})')
		return target_time

	if BUILD_DEBUG:
		print(f'{ESC_AQUA}{target} ({target_time}) is outdated because {outdated} ({dep_time}) is newer{ESC_RST}')

	for cmd in step['cmd']:
		if callable(cmd):
			print(ESC_AQUA + 'call ' + repr(cmd) + ESC_RST)
			cmd()
		else:
			str_cmd = ' '.join(map(str, cmd)) if isinstance(cmd, list) else cmd
			print(ESC_AQUA + '$ ' + str_cmd + ESC_RST)
			res = subprocess.run(list(map(str, cmd)))
			if res.returncode != 0:
				print(f'{ESC_ERR}Error: step terminated with exit code {res.returncode}{ESC_RST}')
				exit(1)

	new_time = os.path.getmtime(target) if os.path.exists(target) else 0
	if dep_time > new_time and not step['phony']:
		print(f'{ESC_ERR}Warning: file `{target}` was not updated by its step{ESC_RST}')

	if BUILD_DEBUG:
		print(f'{ESC_AQUA}{target} new time is {new_time}{ESC_RST}')

	return new_time

def main(default=None):

	if '--help' in sys.argv or '-h' in sys.argv:
		print('A miniature Make-like build system in python')
		print('--------------------------------------------')
		print('Usage: ./build.py [-h|--help] [-l|--list] [TARGETS]')
		print('Makes files specified as TARGET-s')
		print('--help prints this message, --list shows list of all targets avaliable')
		exit(0)

	if '--list' in sys.argv or '-l' in sys.argv:
		for i in steps:
			print(i)
		exit(0)

	if len(sys.argv) > 1:
		for i in sys.argv[1:]:
			refresh(i)
	elif default is not None:
		refresh(default)


###
### Build script itself
###

import glob

BUILD_DIR = 'build'

COMMON_CFLAGS = ['-c', '-Wall', '-g', '-mavx2', '-Isrc/']
COMMON_LDFLAGS = ['-g', '-lSDL2', '-lm']

CCs = [
	[ 'gcc-o2', 'gcc', COMMON_CFLAGS + ['-O2'], COMMON_LDFLAGS ],
	[ 'clang-o2', 'clang', COMMON_CFLAGS + ['-O2'], COMMON_LDFLAGS ],
	[ 'gcc-o3', 'gcc', COMMON_CFLAGS + ['-O3'], COMMON_LDFLAGS ],
	[ 'clang-o3', 'clang', COMMON_CFLAGS + ['-O3'], COMMON_LDFLAGS ],
]

COMMON_SOURCES = glob.glob('src/color/*.c') + glob.glob('src/gen/*.c')
BENCH_SOURCES = glob.glob('src/benchmark/*.c')
VIEWER_SOURCES = glob.glob('src/viewer/*.c')
HEADERS = glob.glob('src/**/*.h', recursive=True)

ALL_SOURCES = COMMON_SOURCES + BENCH_SOURCES + VIEWER_SOURCES

os.makedirs(BUILD_DIR, exist_ok=True)

def sourcename_to_objname(c_file, compiler):
	return os.path.join(
			BUILD_DIR,
			c_file.removeprefix('src/').removesuffix('.c').replace('/', '.')\
					+ '.' + compiler + '.o'
	)

to_clean = []

for cc in CCs:
	(name, cc_cmd, cflags, ldflags) = cc

	get_obj_name = lambda c_file : sourcename_to_objname(c_file, name)
	common_objs = list(map(get_obj_name, COMMON_SOURCES))
	bench_objs = list(map(get_obj_name, BENCH_SOURCES))
	viewer_objs = list(map(get_obj_name, VIEWER_SOURCES))

	to_clean += common_objs + bench_objs + viewer_objs

	for c_file in ALL_SOURCES:
		obj_file = get_obj_name(c_file)
		step(
			out = obj_file,
			deps = [c_file] + HEADERS,
			cmd = [ cc_cmd, *cflags, c_file, '-o', obj_file ]
		)

	viewer_exec = os.path.join(BUILD_DIR, f'viewer-{name}')
	to_clean.append(viewer_exec)
	step(
		out = viewer_exec,
		deps = common_objs + viewer_objs,
		cmd = [ cc_cmd, *ldflags, *common_objs, *viewer_objs, '-o', viewer_exec ]
	)
	
	bench_exec = os.path.join(BUILD_DIR, f'bench-{name}')
	to_clean.append(bench_exec)
	step(
		out = bench_exec,
		deps = common_objs + bench_objs,
		cmd = [ cc_cmd, *ldflags, *common_objs, *bench_objs, '-o', bench_exec ]
	)


step(
	'clean',
	phony=True,
	cmd=[ 'rm', '-f', *to_clean ]
)

step(
	'res/bench.target',
	deps = [f'build/bench-{cc}' for (cc, _, _, _) in CCs],
	cmd = ['./benchmark.sh']
)

step(
	'plot.svg',
	deps = [ 'res/bench.target' ],
	cmd = [ 'python3', './interpret.py' ]
)

if __name__ == '__main__':
	main(default='bench')
