savedcmd_/home/pi/programing/dht11/dht11_driver.mod := printf '%s\n'   dht11_driver.o | awk '!x[$$0]++ { print("/home/pi/programing/dht11/"$$0) }' > /home/pi/programing/dht11/dht11_driver.mod
