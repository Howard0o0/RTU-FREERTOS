# RTU-FREERTOS

# 配置报文: (必须要配置，否则会无限重启)
1.恢复遥测站出厂设置  
7E7E00445566776204D248800A020000191126151933980005C25F0D0A

2.中心站修改遥测站基本配置表 (需要修改设备号,在连续的四个66后的一个字节，并生成CRC16-MODEMBUS校验码，倒数第三第四字节)
7E 7E 00 68 15 33 33 00 12 34 40 80 27 02 00 00 18 01 21 15 00 33 02 28 66 66 66 66 79 04 50 02 03 91 08 19 00 66 00 99 99 05 50 02 04 70 97 21 81 24 00 66 66 05 71 31 0d 0a

3.中心站修改遥测站运行参数配置表 (5分钟发一次)
7E7E006666666601123442803802000019112615193320080121080522080823100300240805250905260901270801282300001000301B0010003812012540120001411200030581B90d0a

4.自定义扩展配置要素报文
7E7E006666666601123442802F020000140121160134FF275001030101050104101002FF605002030101050104101802FF615002030101050105101802051E4A0d0a 

# BUG

## 拔掉调试器后配置报文被reset
拔掉调试器后需要重新4条报文


