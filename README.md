a) Name: XX

b) Student ID: XX

c) 在这个项目中,实现了一个带有身份认证的宿舍管理系统，系统由两个客户端服务器，一个主服务器和三个后端服务器构成。工作流如下：

1. 依次启动各服务器进程，主服务器从各个后端服务器中获取房间信息，构建路由表，然后客户端接受客户来自键盘的输入用户名和密码，之后进行加密将密文发送给主服务器进行验证，如果密码为空则认为用户以guest身份进行登录

2. 主服务器根据磁盘中对应用户注册信息，来进行验证并返回客户端验证信息。

3. 客户端收到主服务器的验证信息，然后根据情况选择让用户继续输入用户名密码或者是输入房间代号进行查询,让用户选择是availiable查询还是reservation请求,然后将代号发送给主服务器。

4.主服务器收到课程代号之后先根据路由表，转发给对应的后端服务器，如果代码不在路由表中，则根据代号第一位进行转发，如果第一位不是S/U/D这三个之一，则直接返回找不到。

5.后端服务器接收到主服务器转发过来的消息代号之后判断有没有该课程，并且根据情况返回信息，根据是否是Reservation来看是否更新数量

6.主服务器收到来自后端的消息之后转发给客户端。

7.客户端呈现给用户查询结果或者订阅结果。


d) 
- client.cpp: Connect to the main server over TCP. Send over the username and password to identify the user. Then, send the roomcode for the query. Show the result on the screen.
- serverM.cpp: Connect to the client over TCP. Connect to the SERVERU, SERVERD, and serverS over UDP. Get the information from the client and send it to the corresponding server and give the result back to the client.
- serverS/U/D: 根据serverM转发过来的roomcode查询房间的状态 然后房间状态转发给主服务器 如果是订阅信息的话则更新房间数量。


e) The output on the screen is the same as requested. Inside the coding files:
- client.cpp:
    - Read the username and password in string format.
    - Send to the main server in char[] format.
    - Get back the result in char[] format.
        - "0" means username not found.
        - "1" means wrong password.
        - "2" means both username and password are correct.
        - "3" means login as guest
    - Read in roomcode in string format.
    - Send to the main server in char[] format.
        - char[0] == 'a' means it is availiable request
        - chat[0] == 'r' means it is reservation request
        - from char[1] to char[n-1] stroes the roomcode 
    - Get back the result in char[] format.
        - when char[1] == 'a'
            - char[0] == "0" means room is available.
            - char[0] == "1" means room is not available.
            - char[0] == "2" means room does not exist.
        - when char[1] == 'r'
            - char[0] == "0" means reservation success.
            - char[0] == "1" means failed.
            - char[0] == "2" means room does not exist.

- serverM:
    - map<string,int> 用于存放用户注册信息
    - All the information sent or received in char[].
    - 收到code 如果房间前缀不在U/D/S范围内，就返回"2"给client
    - 根据路由表将code分发给对应后端服务器
    - 收到char[]格式的结果
        - when char[1] == 'a'
            - char[0] == "0" means room is available.
            - char[0] == "1" means room is not available.
            - char[0] == "2" means room does not exist.
        - when char[1] == 'r'
            - char[0] == "0" means reservation success.
            - char[0] == "1" means failed.
            - char[0] == "2" means room does not exist.

- SERVERU/S/L:
    - map<string,int>用于存放房间信息
    - All the information sent or received in char[].
    - 输入的请求中的 input[0] == 'a' or 'r'，当input[0] == 'r' 的时候 在查询时需要对room数量进行更新。
    - 将结果返回主服务器
        - char [1] = 'a' or 'r' means availiability response or reservation response.
        - when char[1] == 'a'
            - char[0] == "0" means room is available.
            - char[0] == "1" means room is not available.
            - char[0] == "2" means room does not exist.
        - when char[1] == 'r'
            - char[0] == "0" means reservation success.
            - char[0] == "1" means failed.
            - char[0] == "2" means room does not exist.

f) The project works fine in regular conditions. It may crash if the user's input is too long, which is not a normal case.

g) I used some codes from Beej's guide. I also checked the Q&A from the piazza.

h) extra

first install openssl library

```bash
sudo apt-get install openssl
sudo apt-get install libssl-dev
```
先重新进行编译,编译带有多种加密方式的版本。
```bash
#make
make clean
make extra
```
然后执行密码生成脚本，将会生成member_sha256.txt member_md5.txt两个文件，存放着对应的加密后的文本
```bash
./gen
```

内容如下
```bash
# member_md5.txt
d52e32f3a96a64786814ae9b5279fbe5, c4ebb9e8f7b78fe8ee8a8136b1f0db23
e39e74fb4e80ba656f773669ed50315a, 1f6d762806ba3f3bae2cca46e146b3bc
61409aa1fd47d4a5332de23cbf59a36f, aedf38a0c51c0ef104f597183fa13de3
54a7b18f26374fc200ddedde0844f8ec, ac87d8331fe7f86e36579755a6d28ce3
3e06fa3927cbdf4e9d93ba4541acce86, cb9e934a6bbe2fe6134579eaf480bfb6

# memeber_sha256.txt
9345a35a6fdf174dff7219282a3ae4879790dbb785c70f6fff91e32fafd66eab, af5a10777dbd71058a8565665a40a2472200e03a5708c6ccf5e17b500ca81cb1
aebac53c46bbeff10fdd26ca0e2196a9bfc1d19bf88eb1efd65a36151c581051, 385cc817983514f20982ac7d490792e1aa21f65af9ee38f44e348c5f74084be8
a8cfcd74832004951b4408cdb0a5dbcd8c7e52d43f7fe244bf720582e05241da, 7f0842f9d40334c18004b7daa77c5b10f6a228e055a086b5c5dedc05c2580afe
4d8ca8e59c813f36e0321f8b35d0b522a1b9840db6827abed9c90734a6cd31e5, 78191c5c68b7d16643288563e2e786a5bc0d78028f0f68984d3b2d25df5487fb
f089eaef57aba315bc0e1455985c0c8e40c247f073ce1f4c5a1f8ffde8773176, cb4addc378f860b6fb23b2085a84b7c835b28a199a76934e7dccc58e272f9a19

```

然后依次执行 serverM serverS serverD serverU client

这里注意执行serverM的时候有三种选项
```
./serverM
./serverM sha256
./serverM md5
```

然后启动client的时候也有三种选项
```
./client
./client sha256
./client md5
```
注意：serverM和client后面的选项必须要对应，否则无法正常运行。




