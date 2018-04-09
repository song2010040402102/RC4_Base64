#include <stdio.h>
#include <string.h>

const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="; 

char get_index(char ch)   
{ 
	char *ptr = (char*)strrchr(base, ch);
	return (ptr - base); 
}

//RC4±à½âÂë
void RC4(unsigned char *pSrc, int nSrcLen, const char *pKey, int nKeyLen);

//base64±àÂë
void EncodeBinary2String(const void *src, int lenSrc, char* &res, int &lenRes);

//base64½âÂë
void DecodeString2Binary(const char *src, int lenSrc, char* &res, int &lenRes);

//RC4->base64±àÂë
void EncodeRC4Base64(const char* pSrc, char* pDst, const char* pKey);

//base64->RC4½âÂë
int DecodeBase64RC4(const char* pSrc, char* pDst, const char* pKey);

int main(int argc, char *argv[])
{
	char *pSrc = "hello world!";
	char res1[128] = {0}, res2[128] = {0};

	//±àÂë
	EncodeRC4Base64(pSrc, res1, "123456");
	printf("encode, src: %s  -->  res: %s\n", pSrc, res1);

	//½âÂë
	DecodeBase64RC4(res1, res2, "123456");
	printf("decode, src: %s  -->  res: %s\n", res1, res2);
	return 0;	
}

void RC4(unsigned char *pSrc, int nSrcLen, const char *pKey, int nKeyLen)
{	
	const int MAX_SIZE = 10*1024;
	unsigned char S[256] = {0}, T[256] = {0}, R[MAX_SIZE] = {0};
	
	if(!pSrc || !pKey || nSrcLen <=0 || nSrcLen > MAX_SIZE || nKeyLen <=0)
		return;		
	
	int i = 0;
	for(i = 0; i < 256; i++)
	{
		S[i] = i;
		T[i] = pKey[i%nKeyLen];
	}
	
	int j = 0;
	for(i = 0; i < 256; i++)
	{
		j = (j+S[i]+T[i])%256;
		
		unsigned char temp = S[i];
		S[i] = S[j], S[j] = temp;		
	}
	
	i=0, j=0;
	for(int m = 0; m < MAX_SIZE; m++)
	{
		i = (i+1)%256;
		j = (j+S[i])%256;
		
		unsigned char temp = S[i];
		S[i] = S[j], S[j] = temp;
		
		int t = (S[i]+S[j])%256;
		R[m] = S[t];
	}
	
	for(i = 0; i < nSrcLen; i++)
	{
		pSrc[i] ^= R[i];		
	}
}

void EncodeBinary2String(const void *src, int lenSrc, char* &res, int &lenRes)
{
	int cp = (lenSrc%3==0? 0: 3-(lenSrc%3));
	
	lenSrc += cp;
	lenRes = lenSrc*4/3;
	
	unsigned char* newSrc = new unsigned char[lenSrc];
	
	memcpy(newSrc, src, lenSrc);
	int i;
	for(i =0; i< cp; i++)
		newSrc[lenSrc-1-i] = 0;
	
	res = new char[lenRes+1];
	
	for(i= 0; i< lenSrc; i+=3)
	{		
		res[i/3*4] = base[newSrc[i]>>2];
		res[i/3*4 + 1] = base[((newSrc[i]&3) <<4) + (newSrc[i+1]>>4)];
		res[i/3*4 + 2] = base[((newSrc[i+1]&15) <<2) + (newSrc[i+2]>>6)];
		res[i/3*4 + 3] = base[(newSrc[i+2]&63)];
	}
	
	for(i = 0; i< cp; i++)
		res[lenRes-1-i] = '=';
	res[lenRes] = '\0';
	
	delete []newSrc;
	return;
}

void DecodeString2Binary(const char *src, int lenSrc, char* &res, int &lenRes)
{
	lenRes = lenSrc*3/4;
	res = new char[lenRes];
	
	int cp = 0;
	if(src[lenSrc-1] == '='){lenRes--, cp++;}
	if(src[lenSrc-2] == '='){lenRes--, cp++;}
	
	char* newSrc = new char[lenSrc-cp];
	int i;
	for(i = 0; i<lenSrc-cp; i++)
		newSrc[i] = get_index(src[i]);
	
	for(i= 0; i< lenRes; i+=3)
	{
		res[i] = (newSrc[i/3*4]<<2) + (newSrc[i/3*4 + 1]>>4);
		res[i+1] = ((newSrc[i/3*4 + 1]&15) <<4) + (newSrc[i/3*4 + 2]>>2);
		res[i+2] = ((newSrc[i/3*4 + 2]&3) <<6) + (newSrc[i/3*4 + 3]);
	}
	
	delete []newSrc;
	return;
}

void EncodeRC4Base64(const char* pSrc, char* pDst, const char* pKey)
{
	if(!pSrc || !pDst || !pKey)
		return;
	
	int nSrcLen = strlen(pSrc);
	int nKeyLen = strlen(pKey);
	
	char* pSrcCopy = new char[nSrcLen+1];
	memcpy(pSrcCopy, pSrc, nSrcLen+1);
	
	RC4((unsigned char*)pSrcCopy, nSrcLen, pKey, nKeyLen);
	
	int nResLen = 0;
	char *pRes = NULL;	
	EncodeBinary2String(pSrcCopy, nSrcLen, pRes, nResLen);
	
	if(pRes)
	{
		memcpy(pDst, pRes, nResLen);
		pDst[nResLen] = '\0';
		
		delete []pRes, pRes = NULL;
	}	
	
	if(pSrcCopy)
	{
		delete []pSrcCopy, pSrcCopy = NULL;
	}
}

int DecodeBase64RC4(const char* pSrc, char* pDst, const char* pKey)
{
	if(!pSrc || !pDst || !pKey)
		return 0;
	
	int nSrcLen = strlen(pSrc);
	int nKeyLen = strlen(pKey);	
	
	int nResLen = 0;
	char *pRes = NULL;	
	DecodeString2Binary(pSrc, nSrcLen, pRes, nResLen);
	
	if(pRes)
	{
		RC4((unsigned char*)pRes, nResLen, pKey, nKeyLen);
		
		memcpy(pDst, pRes, nResLen);
		pDst[nResLen] = '\0';
		
		delete []pRes, pRes = NULL;
		return nResLen;
	}
	return 0;
}