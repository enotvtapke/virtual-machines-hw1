## Virtual machines HW 1. Cache parameters

Нужно запустить main. Код может исполняться несколько десятков секунд. Если это слишком долго, можно уменьшить `REPEATS` 
в [main.cpp](main.cpp).

Пример вывода:
```
Entity has line size 64
Entity with assoc 6 has entity stride 65536 Bytes and size 393216 Bytes
Entity with assoc 12 has entity stride 4096 Bytes and size 49152 Bytes
```
или
```
Possible entity line size: 64
Possible entities assocs: [10, 48, 72]
Entity with assoc 10 has entity stride 4096 Bytes and size 40960 Bytes.
```

Иногда результаты не полностью правильные.

Если одна из характеристик не была напечатана, программа не смогла её определить. 
Если было напечатано несколько одинаковых характеристик, было зафиксировано несколько сущностей.

Иногда код правильно не определяет характеристики с первый попытки, в таком случае лучше его перезапустить пару раз.

На [analysis.ipynb](analysis.ipynb) можно не обращать внимания.
