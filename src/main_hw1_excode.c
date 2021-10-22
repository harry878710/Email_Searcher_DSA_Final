#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#define maxlen 1000000
const double eps = 1e-15;
int calclen;
char calc[100000][10];
int pff;
char pf[100000][10];
int stkf;
char stk[100000][10];
int suc;
double tmp[maxlen];

void preprocess(char* S)
{
	calclen=0;
	int L=strlen(S);
	for(int i=0;i<L;){
		if(S[i]>='0' && S[i]<='9'){
			char s[10];
			int f=0;
			while(i<L && S[i]>='0' && S[i]<='9'){
				s[f++]=S[i++];
			}
			s[f]='\0';
			strcpy(calc[calclen++],s);
		}
		else{ // +-*/()
			calc[calclen][0]=S[i];
			calc[calclen][1]='\0';
			calclen++;
			i++;
		}
	}
}
int precedence(char* s)
{
	if(strcmp(s,"(")==0 || strcmp(s,")")==0) return 100;
	if(strcmp(s,"*")==0 || strcmp(s,"/")==0) return 2;
	if(strcmp(s,"+")==0 || strcmp(s,"-")==0) return 3;
}
bool isnum(char* s)
{
	if(s[0]>='0' && s[0]<='9') return true;
	return false;
}
void postfix()
{
	stkf=0;
	pff=0;
	for(int i=0;i<calclen;i++){
		if(strcmp(calc[i],"(")==0){
			strcpy(stk[stkf++],calc[i]);
		}
		else if(strcmp(calc[i],")")==0){
			while(stkf && strcmp(stk[stkf-1],"(")!=0){
				strcpy(pf[pff++],stk[stkf-1]);
				stkf--;
			}
			stkf--;
		}
		else if(isnum(calc[i])){
			strcpy(pf[pff++],calc[i]);
		}
		else{
			while(stkf && precedence(stk[stkf-1])<=precedence(calc[i])){
				strcpy(pf[pff++],stk[stkf-1]);
				stkf--;
			}
			strcpy(stk[stkf++],calc[i]);
		}
	}
	while(stkf){
		strcpy(pf[pff++],stk[stkf-1]);
		stkf--;
	}
}
double calculate()
{
	suc=1;
	int tmpf=0;
	for(int i=0;i<pff;i++){
		char* endptr;
		if(isnum(pf[i])) tmp[tmpf++]=1.0*strtod(pf[i],&endptr);
		else{
			double b=tmp[--tmpf];
			double a=tmp[--tmpf];
			//cout << "a=" << a << ", b=" << b << "\n";
			if(strcmp(pf[i],"*")==0) tmp[tmpf++]=1.0*a*b;
			else if(strcmp(pf[i],"/")==0) {
				if(fabs(b-0.0)<eps){
					suc=0;
					return -1;
				}
				tmp[tmpf++]=1.0*a/b;
				//cout << tmp.top() << "\n";
			}
			else if(strcmp(pf[i],"+")==0) tmp[tmpf++]=a+b;
			else if(strcmp(pf[i],"-")==0) tmp[tmpf++]=a-b;
			//cout << "result of a and b = " << tmp.top() << "\n";
		}
	}
	return tmp[tmpf-1];
}
int main()
{
	char S[maxlen];
	int cnt=0;
	while(scanf("%s",S)!=EOF){
		preprocess(S);
		postfix();
		double res=calculate();
		//cout << res.first << " " << res.second << "\n";
		/*if(suc){
			double tmp=res;
			res*=1e6;
			res=round(res);
			if(res==0.) printf("0.000000\n");
			else printf("%.6lf\n",tmp);
		}*/
		printf("%.15lf\n",res);
		cnt++;
	}
}