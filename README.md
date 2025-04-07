![Мандельброт](result.png)

# Быстрое вычисление множества Мандельброта

Здесь реализовано три варианта -- простой, с AVX и с AVX2.
Множество считается на `float`-ах.

 - Простой: [`gen/simple.c`](gen/simple.c)
 - AVX: [`gen/avx.c`](gen/avx.c)
 - AVX2: [`gen/avx2.c`](gen/avx2.c)

## Компиляция

```bash
$ cd viewer
$ make
$ ./viewer
```

Опции:

 - `-b NUM_RUNS`: Запускаем замер времени, делаем `NUM_RUNS` измерений.
 - `-f NUM`: Допустимая разница между максимум и минимум времени, чтобы признать измерение стабильным.
    Например `1.01` допустит, чтобы максимальное время было в `1.01` раза больше, чем минимальное.
 - `-o N`: Выбираем оптимизации: `0` это базовая версия, `1` это AVX, `2` это AVX2.

Без `-b` открывается окно с просмотрщиком.

## Результаты

Замеры производились при выключенном энергосбережении,
без запущенного графического окружения. Все замеры выполнялись последовательно.

Считалось изображение размера `1024 x 768`, с центром `(0, 0)` и шириной окна `2`.
То, что в него попадает видно на изображении сверху.

### Базовая версия

```bash
$ ./viewer -b 32 -f 1.0005
```

```
Done 69 warmup runs and 32 measurment runs
Time avg 361.158966 ms, std dev 0.009283 ms
With 𝜎 (68% probability) time is 361.158966 ± 0.009283 ms
With 3𝜎 (99.73% probability) time is 361.158966 ± 0.027849 ms
Have 0 outliers
Non-outlier runs are withn 2.689𝜎, outliers (>3𝜎) are withn 0.000𝜎
```

Итого, одна `361.158 ± 0.028 ms`.

### С интринсиками AVX

```bash
$ ./viewer -o 1 -b 32 -f 1.0001
```

```
Done 232 warmup runs and 32 measurment runs
Time avg 94.978943 ms, std dev 0.002179 ms
With 𝜎 (68% probability) time is 94.978943 ± 0.002179 ms
With 3𝜎 (99.73% probability) time is 94.978943 ± 0.006538 ms
Have 0 outliers
Non-outlier runs are withn 2.321𝜎, outliers (>3𝜎) are withn 0.000𝜎
```

Получили `94.978 ± 0.007 ms`

### С интринсиками AVX2

```bash
$ ./viewer -o 2 -b 32 -f 1.0005
```

```
Done 25 warmup runs and 32 measurment runs
Time avg 48.829254 ms, std dev 0.003518 ms
With 𝜎 (68% probability) time is 48.829254 ± 0.003518 ms
With 3𝜎 (99.73% probability) time is 48.829254 ± 0.010553 ms
Warn: Run 10 has distance to center 3.338882𝜎
Warn: Run 25 has distance to center 3.054768𝜎
Have 2 outliers
```

Итого, `48.82 ± 0.01 ms`.

## Результат

При AVX ускорение в ~ `3.8` раза, при AVX2 в ~ `7.4` раза.
