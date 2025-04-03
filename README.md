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
 - `-s NUM_START`: Проводим вычисление `NUM_START` раз до начала замера,
   чтобы прогреть процессор, составить вероятности веток, загрузить кэши и прочее.
 - `-o N`: Выбираем оптимизации: `0` это базовая версия, `1` это AVX, `2` это AVX2.

Без `-b` открывается окно с просмотрщиком.

## Результаты

Замеры производились при выключенном энергосбережении,
без запущенного графического окружения.

Считалось изображение размера `1024 x 768`, с центром `(0, 0)` и шириной окна `2`.
То, что в него попадает видно на изображении сверху.

### Базовая версия

```bash
$ ./viewer -b 64 -s 8
```

```
Done 8 warmup runs and 64 measurment runs
Time avg 361.370697 ms, std dev 0.153093 ms
With 𝜎 (68% probability) time is 361.370697 ± 0.153093 ms
With 3𝜎 (99.73% probability) time is 361.370697 ± 0.459279 ms
Have 0 outliers
Non-outlier runs are withn 1.988𝜎, outliers (>3𝜎) are withn 0.000𝜎
```

Итого, одна `361.37 ± 0.46 ms`.

### С интринсиками AVX

```bash
$ ./viewer -o 1 -b 64 -s 8
```

```
Done 8 warmup runs and 64 measurment runs
Time avg 94.805061 ms, std dev 0.227503 ms
With 𝜎 (68% probability) time is 94.805061 ± 0.227503 ms
With 3𝜎 (99.73% probability) time is 94.805061 ± 0.682510 ms
Have 0 outliers
Non-outlier runs are withn 1.886𝜎, outliers (>3𝜎) are withn 0.000𝜎
```

Получили `94.80 ± 0.68 ms`

### С интринсиками AVX2

```bash
$ ./viewer -o 2 -b 64 -s 8
```

```
Done 8 warmup runs and 64 measurment runs
Time avg 48.751766 ms, std dev 0.049411 ms
With 𝜎 (68% probability) time is 48.751766 ± 0.049411 ms
With 3𝜎 (99.73% probability) time is 48.751766 ± 0.148232 ms
Have 0 outliers
Non-outlier runs are withn 1.594𝜎, outliers (>3𝜎) are withn 0.000𝜎
```

Итого, `48.75 ± 0.15 ms`.

## Результат

При AVX ускорение в ~ `3.8` раза, при AVX2 в ~ `7.4` раза.
