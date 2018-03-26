AA一般可以是厂商名、芯片名、芯片的系列名、等等，例如TI OMAP系列的芯片，AAA可以是omap。BBB一般是模块名
基本的platform driver包含三要素：struct platform_driver变量、probe/remove函数、用于和device tree匹配的match table

使用:查找替换
%s/AAA/owl/g, %s/BBB/serial/g
