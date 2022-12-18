#include "client.h"

#pragma comment(lib,"ws2_32.lib")//链接dll

clientState state = clientState::noLogin;//定义全局变量


Client::Client():sendfilePort(14242),getfilePort(14243)
{
    promptMap =
    {
        {clientState::noLogin,"need login> "},
        {clientState::cmdLine,"cmd> "},
        {clientState::isChatting,"chatting with "},//进入chat状态要根据对方的sname来拼接
        {clientState::isWaiting,"waiting> "}
    };
    
    chatSname = "";
    fileNumber = 0;
    prompt = promptMap[state];
    chooseflag = false;
    exitflag = false;

        
}
Client::~Client()
{
    if (isRecvingGf || isRecvingSf)
        cout << "Wait...Some file is still downloading..." << endl;
    while (isRecvingGf || isRecvingSf);//阻塞
}
void Client::run(SOCKET connfd)
{
    cout << "==========================start running==========================" << endl;
    //一个汉字两字节，可能需要更大的buf
    char cmdbuf[256];
    char cmdtmp[256] = "no cmd";
    char recvbuf[256];
    bool reflag = false;
    bool exitflag = false;//函数退出信号
    cout << prompt << flush;
    //根据bool或运算顺序，如果用上一次的结果（reflag==true），就不接收字符
    while (reflag || fgets(cmdbuf, sizeof(cmdbuf), stdin) != NULL)//gets已不被编译器支持，不太安全
    {
        if (chooseflag)//如果在选择文件。接收线程会把这个flag改成true，并且打印提示符enter file number> 
        {
            if (isNumber(cmdbuf))
            {
                //要进行数字的范围判断，用stoi
                int num = stoi(cmdbuf);
                if (num > fileNumber || num < 1)
                {
                    cerr << "Please enter a number in the range！" << endl;
                    memset(cmdbuf, 0, sizeof(cmdbuf));
                    cout << prompt << flush;
                    continue;
                }
                
                
                //...还需要用一个线程来接收
                if (isRecvingGf)//如果线程正在执行，也即正在接收文件，告知本地用户稍后再让对方发送
                {
                    cerr << "One file is being downloaded for getfile, please request another one later！" << endl;
                    string choosecmd = "choosefile 0";//拼接命令，零表示无法接收
                    send(connfd, choosecmd.c_str(), int(choosecmd.size()), 0);//发送
                }
                else
                {
                    string choosecmd = "choosefile " + string(cmdbuf);//拼接命令
                    send(connfd, choosecmd.c_str(), int(choosecmd.size()), 0);//发送
              
                    string filename = config_map["download_path"] + "/" + getResourceMap[num];//拼接完整路径
                    thread(bind(recvfile, getfilePort, filename, 1)).detach();
                }
                memset(cmdbuf, 0, sizeof(cmdbuf));
                chooseflag = false;
                state = clientState::cmdLine;
                prompt = promptMap[state];
                cout << prompt << flush;
                continue;

            }

            else
            {
                cerr << "Please enter a number！" << endl;
                memset(cmdbuf, 0, sizeof(cmdbuf));
                cout << prompt << flush;
                continue;
            }
        }
        if (state == clientState::isChatting && cmdbuf[0] != '@')//如果在chatting，并且输入第一个不是@，判定为聊天内容直接发送
        {
            //send...
            string charstr = "~" + string(cmdbuf);
            send(connfd, charstr.c_str(), int(charstr.size()), 0);//发送
            memset(cmdbuf, 0, sizeof(cmdbuf));
            cout << prompt << flush;
        }
        else//剩下的内容都是else的
        {
            memset(recvbuf, 0, sizeof(recvbuf));//把接收缓冲清零
            string cmdstr(cmdbuf);
            trim(cmdstr);

            if (cmdstr == "\n")//不管怎么样都有个换行，仅有一个换行就不管
            {
                memset(cmdbuf, 0, sizeof(cmdbuf));
                cout << prompt << flush;
                continue;
            }
            vector<string> cmdvec = parse(cmdstr);

            //查找命令
            auto iter = cmdmap.find(cmdvec[0]);
            if (iter == cmdmap.end())//命令错误
            {
                cerr << "Wrong command, your command parsed is: " << cmdvec[0] << endl;
                memset(cmdbuf, 0, sizeof(cmdbuf));
                cout << prompt << flush;
                continue;
            }

            int cmdvalue = iter->second;
            if (!checkCmd(cmdvalue,chatSname))//如果命令与状态不匹配
            {
                memset(cmdbuf, 0, sizeof(cmdbuf));
                reflag = false;//因为可能是调用了re（连续两次login）会导致错误，这里错误要让用户重新输入，否则会崩溃（cmdbuf是空）
                cout << prompt << flush;
                continue;
            }
            switch (cmdvalue)
            {
            case 0:
                if (cmdvec.size() != 4)
                {
                    cerr << "error> The number of parameters for [register] is wrong!" << endl;
                    break;
                }
                else if (cmdvec[2] != cmdvec[3])
                {
                    cerr << "error> The passwords entered twice are not equal!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    //发送cmdstr就不用做头尾空格去除
                    send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                    recv(connfd, recvbuf, sizeof(recvbuf), 0);//同步接收，是阻塞的
                    if (strcmp(recvbuf, "success") == 0)
                        cout << "The registration is successful and you can login!" << endl;
                    else
                        cout << recvbuf << endl;

                    break;
                }

            case 1:
                if (cmdvec.size() != 3)
                {
                    cerr << "error> The number of parameters for [login] is wrong!" << endl;
                    break;
                }

                else
                {
                    //做处理
                    //发送cmdstr就不用做头尾空格去除
                    send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                    recv(connfd, recvbuf, sizeof(recvbuf), 0);//同步接收，是阻塞的              

                    if (strcmp(recvbuf, "success") == 0)
                    {
                        cout << "Login successfully!" << endl;
                        state = clientState::cmdLine;
                        prompt = promptMap[state];
                    }
                    else
                    {
                        cout << recvbuf << endl;
                    }
                    break;
                }

            case 2:
                if (cmdvec.size() != 2)
                {
                    cerr << "error> The number of parameters for [search] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    //发送cmdstr就不用做头尾空格去除
                    send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                    recv(connfd, recvbuf, sizeof(recvbuf), 0);//同步接收，是阻塞的
                    if (strcmp(recvbuf, "faliure") == 0)
                    {
                        cout << "sid does not exist!" << endl;
                    }
                    else
                    {
                        cout << recvbuf << endl;
                    }
                    break;
                }

            case 3:
                if (cmdvec.size() != 2)
                {
                    cerr << "error> The number of parameters for [chat] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    //send...waiting
                    //发送cmdstr就不用做头尾空格去除
                    send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送

                    state = clientState::isWaiting;
                    prompt = promptMap[state];
                    cout << "waiting> Waiting for a response, you can input break or @break to back to command line..." << endl;

                    //响应在接收线程处理！
                    /*
                    Sleep(3000);//阻塞3秒，假装等待连接
                    state = clientState::isChatting;
                    //chatSname = ...
                    prompt = promptMap[state] + chatSname + "> ";
                    */
                    break;
                }

            case 4:
                if (cmdvec.size() != 2)
                {
                    cerr << "error> The number of parameters for [accept] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    //发送cmdstr就不用做头尾空格去除
                    if (reqTable.checkReq(cmdvec[1]) == "chat")
                    {
                        send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                        reqTable.deleteReq(cmdvec[1]);
                        state = clientState::isChatting;
                        promptMap[clientState::isChatting] = "chatting with " + cmdvec[1] + "> ";
                        prompt = promptMap[state];

                    }
                    else
                        cerr << "error> " << cmdvec[1] << " did not send a chat request!" << endl;
                    break;
                }

            case 5:
                if (cmdvec.size() != 2)
                {
                    cerr << "error> The number of parameters for [reject] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    if (reqTable.checkReq(cmdvec[1]) == "chat")
                    {
                        send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                        reqTable.deleteReq(cmdvec[1]);
                    }
                    else
                        cerr << "error> " << cmdvec[1] << " did not send a chat request!" << endl;
                    break;
                }

            case 6:
                if (cmdvec.size() != 1)
                {
                    cerr << "error> The number of parameters for [break] is wrong!" << endl;
                    break;
                }
                else
                {
                    if (state == clientState::cmdLine)//cmdLine不发送
                        break;
                    //做处理，break也需要发送，这样服务器才能知道用户从某请求的等待中退出了
                    send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                    state = clientState::cmdLine;
                    prompt = promptMap[state];
                    break;
                }

            case 7:
                if (cmdvec.size() != 3)
                {
                    cerr << "error> The number of parameters for [send] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                    break;
                }

            case 8:
                if (cmdvec.size() != 3)
                {
                    cerr << "error> The number of parameters for [sendfile] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    //send...waiting
                    //响应在接收线程处理！
                    send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                    state = clientState::isWaiting;
                    prompt = promptMap[state];
                    sendfilename = cmdvec[2];
                    cout << "waiting> Waiting for a response, you can input @break to back to command line..." << endl;
                    break;
                }

            case 9:
                if (cmdvec.size() != 2)
                {
                    cerr << "error> The number of parameters for [acceptfile] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    if (reqTable.checkReq(cmdvec[1]) == "sendfile")
                    {
                        if (isRecvingSf)//如果线程正在执行，也即正在接收文件，告知本地用户稍后再让对方发送，请求表不会删
                        {
                            cerr << "One file is being downloaded for sendfile, please request another one later！" << endl;
                        }
                        else
                        {
                            reqTable.deleteReq(cmdvec[1]);
                            string filename = config_map["download_path"] + "/" + reqTable.getName();//拼接完整路径
                            //...还需要用一个线程来接收，文件名通过请求表维护
                            thread(bind(recvfile, sendfilePort, filename, 0)).detach();
                            send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                        }
                            
                    }
                    else
                        cerr << "error> " << cmdvec[1] << " did not send a sendfile request!" << endl;


                    break;
                }

            case 10:
                if (cmdvec.size() != 2)
                {
                    cerr << "error> The number of parameters for [rejectfile] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    if (reqTable.checkReq(cmdvec[1]) == "sendfile")
                    {
                        send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                        reqTable.deleteReq(cmdvec[1]);
                    }
                    else
                        cerr << "error> " << cmdvec[1] << " did not send a sendfile request!" << endl;
                    break;
                }

            case 11:
                if (cmdvec.size() != 2)
                {
                    cerr << "error> The number of parameters for [getfile] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    //send...waiting
                    //响应在接收线程处理！
                    send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                    state = clientState::isWaiting;
                    prompt = promptMap[state];
                    cout << "waiting> Waiting for a response, you can input @break to back to command line..." << endl;
                    break;
                }

            case 12:
                if (cmdvec.size() != 2)
                {
                    cerr << "error> The number of parameters for [acceptget] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    //响应在接收线程处理！
                    if (reqTable.checkReq(cmdvec[1]) == "getfile")
                    {
                        //还要发资源列表
                        //cmdstr需要以\n结尾的，这样发过去的字符串服务器用复用这里的解析函数
                        string str = "";
                        if (myresource_str == "")
                        {
                            cerr << "error> " << " There are no resources!" << endl;
                            str = "none";
                        }
                        else
                        {
                            str = myresource_str;
                        }
                        cmdstr = cmdvec[0] + " " + cmdvec[1] + " " + str + "\n";
                        send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                        reqTable.deleteReq(cmdvec[1]);
                    }
                    else
                        cerr << "error> " << cmdvec[1] << " did not send a getfile request!" << endl;
                    break;
                }

            case 13:
                if (cmdvec.size() != 2)
                {
                    cerr << "error> The number of parameters for [rejectget] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    if (reqTable.checkReq(cmdvec[1]) == "getfile")
                    {
                        send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                        reqTable.deleteReq(cmdvec[1]);
                    }
                    else
                        cerr << "error> " << cmdvec[1] << " did not send a getfile request!" << endl;
                    break;
                }

            case 14:
                if (cmdvec.size() != 2)
                {
                    cerr << "error> The number of parameters for [setsid] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                    //需要接收响应，因为sid是全局唯一的，可能设置失败！
                    recv(connfd, recvbuf, sizeof(recvbuf), 0);//同步接收，是阻塞的
                    cout << recvbuf << endl;
                    break;
                }

            case 15:
                if (cmdvec.size() != 2)
                {
                    cerr << "error> The number of parameters for [setsname] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                    //这里接收响应是为了增加用户体验，服务器返回一个成功信息
                    //因为设置相对频率较少，这不是什么很大的负担
                    recv(connfd, recvbuf, sizeof(recvbuf), 0);//同步接收，是阻塞的
                    cout << recvbuf << endl;
                    break;
                }

            case 16:
                if (cmdvec.size() != 1)
                {
                    cerr << "error> The number of parameters for [re] is wrong!" << endl;
                    reflag = false;//虽然是re，但是命令有问题
                    break;
                }
                else//即使是错误命令也存，用户可能头铁...就想再试一次
                {
                    //做处理
                    if (!strcmp(cmdtmp, "no cmd"))//如果还没有命令
                    {
                        cerr << "error> No command yet!" << endl;
                    }
                    else
                    {
                        cout << "reEX> " << cmdtmp;//不用换行，cmdtmp自然有个'\n'
                        //strcpy已不可用，不指定长度不安全。使用strlen要+1，因为长度不包含结束符，要补上去
                        strcpy_s(cmdbuf, strlen(cmdtmp) + 1, cmdtmp);
                        reflag = true;//表明不必把cmdbuf置零
                    }
                    break;
                }

            case 17:
                if (cmdvec.size() != 1)
                {
                    cerr << "error> The number of parameters for [exit] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                    exitflag = true;
                    cout << "The client now exits!" << endl;
                    Sleep(1);
                    break;
                }
                
            case 18:
                if (cmdvec.size() != 2)
                {
                    cerr << "error> The number of parameters for [hisir] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                    recv(connfd, recvbuf, sizeof(recvbuf), 0);//同步接收，是阻塞的
                    cout << recvbuf << endl;
                    break;
                }

            case 19:
                if (cmdvec.size() != 1)
                {
                    cerr << "error> The number of parameters for [myresource] is wrong!" << endl;
                    break;
                }
                else
                {
                    //只要打印就好了
                    string myresolist;
                    for (int index = 1; index < myresource_map.size(); index += 1)
                    {
                        myresolist += "[" + to_string(index) + "]" + "\t" + myresource_map[index] + "\n";
                    }
                    cout << myresolist << flush;
                    break;
                }

            case 20:
                if (cmdvec.size() != 1)
                {
                    cerr << "error> The number of parameters for [oyasumi] is wrong!" << endl;
                    break;
                }
                else
                {
                    //做处理
                    send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
                    recv(connfd, recvbuf, sizeof(recvbuf), 0);//同步接收，是阻塞的
                    cout << recvbuf << endl;
                    exitflag = true;
                    break;
                }

            }

            //是否退出
            if (exitflag)
                break;

            if (cmdvalue == 16 && reflag)//如果是re且成功的话就不用cmd，不用cmdbuf置零
                continue;
            else if (cmdvalue == 16)//虽然是re但失败了，不能保存re
            {
                memset(cmdbuf, 0, sizeof(cmdbuf));
                cout << prompt << flush;
            }
            else
            {
                strcpy_s(cmdtmp, strlen(cmdbuf) + 1, cmdbuf);//暂存上一次命令，方便re调用，如果上一次是re的话，不更新
                memset(cmdbuf, 0, sizeof(cmdbuf));
                cout << prompt << flush;
                reflag = false;//上一次不是re自然置false
            }
        }


    }
}



//解析接收命令，返回解析成不成功，失败则false
bool Client::parseRecv(string cmdrecv)
{
    //最后加一个空格，这样能解析成功
    printf("%s\n", cmdrecv.c_str());
    cmdrecv += " ";
    vector<string> res;
    size_t pos = 0;
    size_t pos1;
    while ((pos1 = cmdrecv.find(' ', pos)) != string::npos)
    {
        res.push_back(cmdrecv.substr(pos, pos1 - pos));
        while (cmdrecv[pos1] == ' ')//过滤空格
            pos1++;
        pos = pos1;
    }

    if (res.size() < 2)
    {
        //cerr << endl << "An error command was received in the receThread!" << endl;非线程安全
        printf("\nAn error command was received in the receThread!\n");
        return false;
    }

    if (res[0] == "chat" && state == clientState::isWaiting)//必须还在等待
    {
        if (res[1] == "accept")
        {
            //cout << endl << "The peer agrees to chat!" << endl;非线程安全
            printf("\nThe peer agrees to chat!\n");
            state = clientState::isChatting;
            chatSname = res[2];//假设服务器把对方sname发来
            promptMap[clientState::isChatting] = "chatting with " + chatSname + "> ";
            prompt = promptMap[state];
        }
        else if (res[1] == "reject")
        {
            //cout << endl << "The peer disagrees to chat!" << endl;非线程安全
            printf("\nThe peer disagrees to chat!\n");
            state = clientState::cmdLine;
            prompt = promptMap[state];
        }
        else
        {
            //cerr << endl << "An error command was received in the receThread!" << endl;非线程安全
            printf("\nAn error command was received in the receThread!\n");
            return false;
        }
    }
    else if (res[0] == "sendfile" && state == clientState::isWaiting)
    {
        if (res[1] == "accept")
        {
            //cout << endl << "The peer agrees to receive file!" << endl;非线程安全
            printf("\nThe peer agrees to receive file!\n");
            state = clientState::cmdLine;
            prompt = promptMap[state];
            //开线程发送
            //假设res[2]是ip，发送到对方的sendfilePort
            thread(bind(sendfile, res[2].c_str(), sendfilePort, sendfilename)).detach();//发送是可以detach的
            printf("%s", res[2].c_str());
        }
        else if (res[1] == "reject")
        {
            //cout << endl << "The peer disagrees to receive file!" << endl; 非线程安全
            printf("\nThe peer disagrees to receive file!\n");
            state = clientState::cmdLine;
            prompt = promptMap[state];
        }
        else
        {
            //cerr << endl << "An error command was received in the receThread!" << endl;非线程安全
            printf("\nAn error command was received in the receThread!\n");
            return false;
        }
    }
    else if (res[0] == "getfile" && state == clientState::isWaiting)
    {
        if (res[1] == "accept" && res[2] == "none")//如果没有资源但同意了，第三个参数就是none
        {
            //cerr << endl << "The peer has no resources!" << endl;非线程安全
            printf("\nThe peer has no resources!\n");
            state = clientState::cmdLine;
            prompt = promptMap[state];
        }
        else if (res[1] == "accept")
        {
            chooseflag = true;
            prompt = "enter the number of file> ";
            //cout << endl << "The peer agrees to getfile!" << endl;非线程安全
            printf("\nThe peer agrees to getfile!\n");
            getResourceMap.clear();//清空
            fileNumber = 0;//重置
            string resolist;//打印信息
            //假设对方发送的文件资源表是[1] some.txt [2] some2.txt，即空格分隔的成对的
            for (int i = 2; i < res.size(); i += 2)
            {
                fileNumber++;
                getResourceMap[fileNumber] = res[i + 1];
                resolist += res[i] + "\t" + res[i + 1] + "\n";
            }
            cout << resolist << flush;


        }
        else if (res[1] == "reject")
        {
            //cout << endl << "The peer doesn't allow you to get the resource!" << endl;非线程安全
            printf("\nThe peer doesn't allow you to get the resource!\n");
            state = clientState::cmdLine;
            prompt = promptMap[state];
        }

        else
        {
            //cerr << endl << "An error command was received in the receThread!" << endl;非线程安全
            printf("\nAn error command was received in the receThread!\n");
            return false;
        }
    }
    else if (res[0] == "choosefile")
    {
        int num = stoi(res[1]);
        if (num == 0)//0表示对方无法接收
        {
            //cout << endl << "The peer is busy, cannot receive files!" << endl;非线程安全
            printf("\nThe peer is busy, cannot receive files!\n");
        }
            
        else
        {
            //cout << endl << "The peer requested the [" + res[1] + "] file!" << endl;非线程安全
            printf("\nThe peer requested the [%d] file!\n", num);
            string filename = config_map["resource_path"] + "/" + myresource_map[num];//拼接完整路径

            //开一个线程发送
            //假设服务器把ip放在res[2]，发送到对方的getfilePort
            thread(bind(sendfile, res[2].c_str(), getfilePort, filename)).detach();//发送是可以detach的
        }
        
    }
    //---------------------------------------------------------------------------------
    //-----------------------------请求表相关命令----------------------------------------
    else if (res[0] == "chatfrom")
    {
        reqTable.insertReq(res[1], "chat");
        //cout << endl << "Receive a chat request from " << res[2] << " -sid:" << res[1] << endl;非线程安全
        printf("\nReceive a chat requestion from [%s] -sid:[%s]\n", res[2].c_str(), res[1].c_str());
    }
    else if (res[0] == "sendfilefrom")
    {
        string req = string("sendfile") + res[3];
        reqTable.insertReq(res[1], req);
        //cout << endl << "Receive a sendfile request from " << res[2] << " -sid: " << res[1] << " -file name: " << res[3] << endl;非线程安全
        printf("\nReceive a sendfile requestion from [%s] -sid:[%s] -file_name:[%s]\n", res[2].c_str(), res[1].c_str(),res[3].c_str());
    }
    else if (res[0] == "getfilefrom")
    {
        reqTable.insertReq(res[1], "getfile");
        //cout << endl << "Receive a getfile request from " << res[2] << " -sid:" << res[1] << endl;非线程安全
        printf("\nReceive a getfilt requestion from [%s] -sid:[%s]\n", res[2].c_str(), res[1].c_str());
    }
    //----break-----
    else if (res[0] == "break")
    {
        if (res[1] == "now")
        {
            if (state == clientState::isChatting)//进入虚假chatting，让其退出
            {
                //cout << endl << "The peers break before you accept!" << endl;非线程安全
                printf("\nThe peers break or exit!\n");//也可能是真chatting
                state = clientState::cmdLine;
                prompt = promptMap[state];
            }
            else//进入接收或发送
            {
                //cout << endl << "The peers break before you accept!" << endl;非线程安全
                printf("\nThe peers break before you accept!\n");
                //要让接收或发送线程退出
                SOCKET connfd = connect_S("127.0.0.1", sendfilePort);//用一个连接使线程退出
                closesocket(connfd);
            }
        }

        else//这种情况是break [sid]，用户sid撤回请求
            reqTable.deleteReq(res[1]);
    }
    else
        return false;
    return true;
}

void Client::recvThread(const char* server_ip, const int port)
{
    //连接服务器
    SOCKET connfd = connect_S(server_ip, port);//获取套接字
    if (connfd == INVALID_SOCKET || connfd == SOCKET_ERROR)
        return;//连接失败退出，错误由connect_S函数输出

    //--------------------------工作-----------------------------
    char recvbuf[256];
    while (!exitflag)
    {
        memset(recvbuf, 0, sizeof(recvbuf));//每次都将buffer清空，防止被上次写入的结果影响
        int recvbytes = recv(connfd, recvbuf, sizeof(recvbuf), 0);//同步接收，是阻塞的
        //异常结束时，退出
        if (recvbytes <= 0)
        {
            //printf("receive thread exits!\n");
            //printf("error: %d",WSAGetLastError());
            break;
        }

        if (recvbytes >= 3 && recvbuf[0] == '@' && recvbuf[1] == '#')//这种情况下要解析命令
        {
            string cmdrecv = recvbuf;
            if (parseRecv(cmdrecv.substr(2, cmdrecv.size() - 2)))
                cout << prompt << flush;//失败就不打印刷新
        }
            
        else//其他信息直接输出
        {
            //cout << endl << recvbuf << endl;非线程安全
            printf("\n%s\n", recvbuf);
            cout << prompt << flush;
        }


    }
    //退出
    closesocket(connfd);
}



void Client::setConfig()
{
    const char* configfile = "./config.txt";
    ifstream cfgFile(configfile, ios::in);

    string line;
    if (cfgFile.is_open())//如果可以打开，则获取参数
    {
        while (getline(cfgFile, line)) //逐行读取，直到结束
        {
            size_t pos = line.find(' ', 0);//找到空格位置
            string key = line.substr(0, pos);//获取子串
            string value = line.substr(pos + 1, line.size() - pos - 1);

            if (value.size() != 0)//如果确实配置了
            {
                config_map[key] = value;//添加映射表项
                //输出配置
                cout << "configuration of [" << key << "]" << " is " << value << endl;
            }
        }

        cfgFile.close();
    }
    //如果没有就默认
    if (config_map.find("resource_path") == config_map.end())
    {
        config_map["resource_path"] = "./resource";
        cout << "default configuration of [resource_path]" << " is " << "./resource" << endl;

        if (!dirExists("./resource"))//不存在就创建
        {
            _mkdir("./resource");
        }
    }
    else if (!dirExists(config_map["resource_path"]))//无法创建多级文件夹，直接报错
        cout << "Directory " << config_map["resource_path"] << " does not exist!" << endl;

    if (config_map.find("download_path") == config_map.end())
    {
        config_map["download_path"] = "./download";
        cout << "default configuration of [download_path]" << " is " << "./download" << endl;
        if (!dirExists("./download"))//不存在就创建
        {
            _mkdir("./download");
        }
    }
    else if(!dirExists(config_map["download_path"]))//无法创建多级文件夹，直接报错
        cout << "Directory " << config_map["download_path"] << " does not exist!" << endl;
}

//传入config_map["resource_path"]，完整文件路径为config_map["resource_path"]+'/'+myresoure_map[i]
//设置发送字符串myresource_str 以及映射表myresoure_map
void Client::genResource(string dirpath)
{

    if (!dirExists(dirpath))
    {
        myresource_str = "";//没有文件夹
        return;
    }
    WIN32_FIND_DATAA fileInfo;
    dirpath = dirpath + "/*";//通配符匹配所有的文件

    //通常，最初的两次搜索得到的文件名为："." 、".."，分别代表当前目录和上级目录，这里过滤掉"."然后进while过滤".."
    HANDLE hFile = FindFirstFileA(dirpath.c_str(), &fileInfo);//现在定位到"."

    if (hFile == INVALID_HANDLE_VALUE) {
        myresource_str = "";//出错
        return;
    }


    FindNextFileA(hFile, &fileInfo);//现在定位到".."
    int index = 1;
    while (FindNextFileA(hFile, &fileInfo))//再把".."过滤掉
    {
        myresource_str += "[" + to_string(index) + "] " + fileInfo.cFileName + " ";
        myresource_map[index++] = fileInfo.cFileName;

    }
    FindClose(hFile);//关闭
}

void Client::autoLogin(SOCKET connfd)
{
    if (config_map.find("user_id") == config_map.end() || config_map.find("user_password") == config_map.end())
        return;//如果有一个没填就不管

    cout << "Login  automatically......" << endl;
    string cmdstr = "login " + config_map["user_id"] + " " + config_map["user_password"] + "\n";
    char recvbuf[128];
    memset(recvbuf, 0, sizeof(recvbuf));
    send(connfd, cmdstr.c_str(), int(cmdstr.size()), 0);//发送
    recv(connfd, recvbuf, sizeof(recvbuf), 0);//同步接收，是阻塞的              

    if (strcmp(recvbuf, "success") == 0)
    {
        cout << "Login successfully!" << endl;
        state = clientState::cmdLine;
        prompt = promptMap[state];
    }
    else
    {
        cout << recvbuf << endl;
    }
}

void Client::start(const char* server_ip, const int port1, const int port2)
{
    initSocket();
    setConfig();
    genResource(config_map["resource_path"]);

    cout << endl << "connecting to server..." << endl;

    //std::bind()绑定类成员函数时，需要在类名前面添加取地址符，让其拥有callable的属性。
    //类成员函数不像普通函数一样可以隐式转换成函数指针，所以需要手动加&，普通函数可以隐式转换
    //别忘了传入this指针
    thread(bind(&Client::recvThread, this, server_ip, port2)).detach();//start本身也是类成员函数，函数不需要指定类名

    //连接服务器
    SOCKET connfd = connect_S(server_ip, port1);//获取套接字
    if (connfd == INVALID_SOCKET || connfd == SOCKET_ERROR)
        return;//连接失败退出，错误由connect_S函数输出
    autoLogin(connfd);
    run(connfd);
    //退出
    closesocket(connfd);
    WSACleanup();
}

