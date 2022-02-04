adb -s 374b816d shell "su -c setenforce 0"
adb -s 374b816d push cmake-build-release\server /data/local/tmp/server
adb -s 374b816d shell "su -c chmod 777 /data/local/tmp/server"

adb -s 374b816d push cmake-build-release\client /data/local/tmp/client
adb -s 374b816d shell "su -c chmod 777 /data/local/tmp/client"

pause