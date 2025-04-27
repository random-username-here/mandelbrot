import os
import re
from dataclasses import dataclass
import matplotlib.pyplot as plt
import numpy as np

RESULTS_DIR = 'res'
RESULT_TIME_RE = re.compile(r'\(68% probability\) time is (\d+(?:\.\d+)?)\s+±\s+(\d+(?:\.\d+))\s+ms')

@dataclass
class Result:
	runs : int = 0
	sum_times : float = 0
	sum_dev_squared : float = 0

results = {}

for name in os.listdir(RESULTS_DIR):
	path = os.path.join(RESULTS_DIR, name)

	(build_name, method, run_number, _) = name.split('.')
	time = 0
	omega = 0
	with open(path, 'r') as f:
		time_match = RESULT_TIME_RE.search(f.read())
		if time_match is None:
			raise ValueError(f'Could not find run time in file `{path}`')
		time = float(time_match.group(1))
		omega = float(time_match.group(2))
	

	key = (build_name, method)

	if key not in results:
		results[key] = Result()

	res = results[key]
	res.runs += 1
	res.sum_times += time
	res.sum_dev_squared += omega * omega


COMPILERS = ['gcc-o2', 'gcc-o3', 'clang-o2', 'clang-o3']
NAMES = [ 'GCC -O2', 'GCC -O3', 'Clang -O2', 'Clang -O3' ]
COLORS = ['#00cec9', '#0984e3', '#fdcb6e', '#e17055']

print('Table:')
print()
print(f'| {"Build":10} | {"Method":10} | {"Avg. time":10} | {"Omega":10} |')
print(('|' + '-'*12) * 4 + '|')

by_method = {}
for (build_name, method), result in results.items():
	avg = result.sum_times / result.runs
	omega = result.sum_dev_squared ** 0.5 / result.runs

	if method not in by_method:
		by_method[method] = [0] * len(COMPILERS)
	by_method[method][COMPILERS.index(build_name)] = avg

	print(f'| {build_name:10} | {method:10} | {avg:10.5f} | {omega:10.5f} |')


BAR_WIDTH = 0.2 # 1/5


fig, ax = plt.subplots(layout='constrained', figsize=(8,6))
multiplier = 0
x = np.arange(len(NAMES))

for (method, measurement), color in zip(by_method.items(), COLORS):
	offset = BAR_WIDTH * multiplier
	rects = ax.bar(x + offset, measurement, BAR_WIDTH, label=method, color=color)
	ax.bar_label(rects, padding=10, rotation=90)
	multiplier += 1

ax.set_ylabel('Time per frame, ms')
ax.set_title('Speed comparsion')
ax.legend()
ax.set_xticks(x + BAR_WIDTH * (len(by_method) - 1)/2, NAMES)
ax.set_ylim((0, ax.get_ylim()[1] * 1.2)) # so labels will fit, not the best solution

fig.savefig('plot.svg')
