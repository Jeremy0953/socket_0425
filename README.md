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

