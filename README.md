# RC4_Base64
对加密后的文本进行网络传输，若加密级别不高，通常先通过RC4加密再Base64转码，接收方进行Base64解码再通过RC4解密
这里从底层提供了RC4的加密函数和Base64编解码函数
