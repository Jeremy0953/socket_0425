a) Name: XX

b) Student ID: XX

c) 在这个项目中,实现了一个带有身份认证的图书管理系统，系统由一个客户端服务器，一个主服务器和三个后端服务器构成。工作流如下：

1. 依次启动各服务器进程，然后客户端接受客户来自键盘的输入用户名和密码，之后进行加密将密文发送给主服务器进行验证

2. 主服务器根据磁盘中对应用户注册信息，来进行验证并返回客户端验证信息。

3. 客户端收到主服务器的验证信息，然后根据情况选择让用户继续输入用户名密码或者是输入课程代号进行查询,然后将代号发送给主服务器。

4.主服务器收到课程代号之后根据代号第一位来进行消息转发，转发给对应的后端服务器，如果代号第一位并不是HSL这三种之一则直接返回找不到。

5.后端服务器接收到主服务器转发过来的消息代号之后判断有没有该课程 并且根据情况返回信息。

6.主服务器收到来自后端的消息之后转发给客户端。

7.客户端呈现给用户查询结果。


d) 
- client.cpp: Connect to the main server over TCP. Send over the username and password to identify the user. Then, send the bookcode for the query. Show the result on the screen.
- serverM.cpp: Connect to the client over TCP. Connect to the serverH, serverL, and serverS over UDP. Get the information from the client and send it to the corresponding server and give the result back to the client.
- serverS/H/L: 根据serverM转发过来的bookcode查询书本的状态 然后图书状态转发给主服务器


e) The output on the screen is the same as requested. Inside the coding files:
- client.cpp:
    - Read the username and password in string format.
    - Send to the main server in char[] format.
    - Get back the result in char[] format.
        - "0" means username not found.
        - "1" means wrong password.
        - "2" means both username and password are correct.
    - Read in bookcode in string format.
    - Send to the main server in char[] format.
    - Get back the result in char[] format.
        - "0" means book is available.
        - "1" means book is not available.
        - "2" means book does not exist.
        

- serverM:
    - map<string,int> 用于存放用户注册信息
    - All the information sent or received in char[].
    - 收到code 如果这本书前缀不在HSL范围内，就返回"2"给client
    - 将code分发给对应后端服务器
    - 收到char[]格式的结果
        - "0" means book is available.
        - "1" means book is not available.
        - "2" means book does not exist.

- serverH/S/L:
    - map<string,int>用于存放图书信息
    - All the information sent or received in char[].
    - 根据收到的code以及自身的map,将结果返回主服务器
        - "0" means book is available.
        - "1" means book is not available.
        - "2" means book does not exist.

g) The project works fine in regular conditions. It may crash if the user's input is too long, which is not a normal case.

h) I used some codes from Beej's guide. I also checked the Q&A from the piazza.

