// BIGDATA.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>

#include <fstream>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
#include<Windows.h>
using namespace std;
//一共有0~19834个用户（19835），但很多函数循环都从int i=0开始，故赋值为19834
int NumOfUser = 19835;

//存放每个用户看过的电影数目
int MovieNum[19835];
int ReadBuffer[19835][6];

struct MovieScore
{
	int IDOfMovie;
	double ScoreOfMovie;
};

//创建数据的效用矩阵
//开19834行（用户）、不固定列的矩阵Data和PearData
MovieScore **Data = new MovieScore *[NumOfUser];
MovieScore **PearData = new MovieScore *[NumOfUser];

bool cmp2(MovieScore a, MovieScore b)
{
	return a.IDOfMovie < b.IDOfMovie;//按照学号升序排列
}

//二分查找
int BinSearch(MovieScore **R, int Uid, int n, int K)
{
	//在有序表R[0..n-1]中进行二分查找，成功时返回结点的位置，失败时返回-1
	int low = 0, high = n - 1, mid;     //置当前查找区间上、下界的初值
	while (low <= high)
	{
		if (R[Uid][low].IDOfMovie == K)
			return low;
		if (R[Uid][high].IDOfMovie == K)
			return high;          //当前查找区间R[low..high]非空
		mid = low + ((high - low) / 2);
		/*使用(low+high)/2会有整数溢出的问题
		（问题会出现在当low+high的结果大于表达式结果类型所能表示的最大值时，
		这样，产生溢出后再/2是不会产生正确结果的，而low+((high-low)/2)
		不存在这个问题*/
		if (R[Uid][mid].IDOfMovie == K)
			return mid;             //查找成功返回
		if (R[Uid][mid].IDOfMovie<K)
			low = mid + 1;              //继续在R[mid+1..high]中查找
		else
			high = mid - 1;             //继续在R[low..mid-1]中查找
	}
	if (low>high)
		return -1;//当low>high时表示所查找区间内没有结果，查找失败
}

//读取数据到Data矩阵
void load()
{
	//打开train.txt
	ifstream infile1;
	infile1.open("train.txt");
	assert(infile1.is_open());   //若失败,则输出错误消息,并终止程序运行 


								 //向data效用矩阵内存数据

	string s; //存放读取行数据
	string blank = " ";
	string line = "|";
	int UserID = -1; //user递增，可以直接先赋值为-1

	int NumOfMovie = 0; //目前的用户所看过的电影数目
	int clock = 0;

	while (getline(infile1, s))//逐行读取数据
	{
		int LocLine = s.find(line); //查找该行中"|"所在位置
		if (s == "")
			break;

		if (LocLine != -1) //"|"存在，新的用户，读取他看过的电影数
		{
			UserID++;
			clock = 0;

			int len = s.length();
			string ss = s.substr(LocLine + 1, len - LocLine);
			NumOfMovie = stoi(ss); //该用户看过的电影数目

			MovieNum[UserID] = NumOfMovie;

			//动态初始化data数组中该UserID行
			Data[UserID] = new MovieScore[NumOfMovie];
		}

		else //"|"不存在，还是上一个用户,现在s存的是电影号和评分
		{
			int LocBlack = s.find(blank); //空格所在位置
			string IDOfMovie = s.substr(0, LocBlack);
			int leng = s.length();
			string sco = s.substr(LocBlack + 2, leng - LocBlack);

			double score = stof(sco);//当前用户对目前行的电影的评分
			int MovieID = stoi(IDOfMovie);//当前电影编号

			struct MovieScore MS = { MovieID,score };

			//将该行数据存入效用矩阵对应位置			
			Data[UserID][clock] = MS;
			clock++;
		}
	}
	infile1.close();
	cout << "traindata.txt已读取完毕" << endl;
}

//将每个用户看过的电影按编号从小到大升序排列
void SortMovieID()
{
	for (int i = 0; i < NumOfUser; i++)
	{
		sort(Data[i], Data[i] + MovieNum[i], cmp2);
	}
}

//求总体平均值
double AvgAll()
{
	double Average = 0;
	double Sum = 0;
	int count = 0;

	for (int i = 0; i < NumOfUser; i++)
	{
		for (int j = 0; j < MovieNum[i]; j++)
		{
			Sum = Sum + Data[i][j].ScoreOfMovie;
			count++;
		}
	}

	Average = Sum / count;
	return Average;
}

//求用户x对所有产品打分的平均值
double AvgUser(int UserID)
{
	double Average = 0;
	double Sum = 0;
	for (int i = 0; i < MovieNum[UserID]; i++)
	{
		Sum = Sum + Data[UserID][i].ScoreOfMovie;
	}
	Average = Sum / MovieNum[UserID];
	return Average;
}

//求产品i的所有评分的平均值
double AvgItem(int ItemID)
{
	double Average = 0;
	double Sum = 0;
	int count = 0;

	for (int i = 0; i < NumOfUser; i++) //外层遍历所有用户
	{
		int j = BinSearch(Data, i, MovieNum[i], ItemID);
		//for (int j = 0; j < MovieNum[i]; j++) //内层遍历每一用户看过的所有电影
		//{
		if (j != -1)
		{
			Sum = Sum + Data[i][j].ScoreOfMovie;
			count++;
		}
		//}
	}

	Average = Sum / count;
	return Average;
}

//最后产品得分中的bxi的值
double bxi(int UserID, int ItemID, double u)
{
	double bx = AvgUser(UserID) - u;
	double bi = AvgItem(ItemID) - u;

	double bxi = u + bx + bi;

	return bxi;
}

//将Data矩阵的数据转成Pearson系数矩阵
void Pearson()
{
	for (int i = 0; i < NumOfUser; i++)
	{
		double avguser = AvgUser(i);
		PearData[i] = new MovieScore[MovieNum[i]];

		for (int j = 0; j < MovieNum[i]; j++)
		{
			PearData[i][j].IDOfMovie = Data[i][j].IDOfMovie;
			PearData[i][j].ScoreOfMovie = Data[i][j].ScoreOfMovie - avguser;
		}
	}
}

//求用户A和用户B的Pearson相似度
double PearSimAB(int UserA, int UserB)
{
	//求Pearson系数的分子

	//求出AB都看过的电影a的编号
	//计算：用户A对电影a的评分*用户B对电影a的评分，存在numerator
	double numerator[10000];
	int clock = 0;

	if (MovieNum[UserA] <= MovieNum[UserB])
	{
		for (int i = 0; i < MovieNum[UserA]; i++) //外层遍历A用户看过的电影
		{
			int j = BinSearch(PearData, UserB, MovieNum[UserB], PearData[UserA][i].IDOfMovie);
			if (j != -1)
			{
				numerator[clock] = PearData[UserA][i].ScoreOfMovie * PearData[UserB][j].ScoreOfMovie;
				clock++;
			}
		}
	}

	if (MovieNum[UserA] > MovieNum[UserB])
	{
		for (int i = 0; i < MovieNum[UserB]; i++) //外层遍历B用户看过的电影
		{
			int j = BinSearch(PearData, UserA, MovieNum[UserA], PearData[UserB][i].IDOfMovie);
			if (j != -1)
			{
				numerator[clock] = PearData[UserB][i].ScoreOfMovie * PearData[UserA][j].ScoreOfMovie;
				clock++;
			}
		}
	}

	//将numerator数组里的各数字求和，作为Pearson系数的分子
	double PeaNumerator = 0;
	for (int i = 0; i < clock; i++)
		PeaNumerator = PeaNumerator + numerator[i];

	//求Pearson系数的分母

	//①求用户A对所有电影的评分各自平方求和开根号
	double SumUserA = 0;
	for (int i = 0; i < MovieNum[UserA]; i++)
		SumUserA = SumUserA + PearData[UserA][i].ScoreOfMovie * PearData[UserA][i].ScoreOfMovie;
	double SqrtUserA = sqrt(SumUserA);

	//②求用户B对所有电影的评分各自平方求和开根号
	double SumUserB = 0;
	for (int i = 0; i < MovieNum[UserB]; i++)
		SumUserB = SumUserB + PearData[UserB][i].ScoreOfMovie * PearData[UserB][i].ScoreOfMovie;
	double SqrtUserB = sqrt(SumUserB);


	//求Pearson系数
	double pearson = PeaNumerator / (SqrtUserA * SqrtUserB);

	return pearson;
}

//最后产品得分中的rxi的值
double rxi(int UserID, int ItemID, double u)
{
	//①求所有"PearSimAB与目标用户作用大于0、且看过目标电影"的其他用户与目标用户的相似度之和
	//②求(看过目标电影的其他用户与目标用户的相似度)*(看过目标电影的其他用户对目标电影的打分)之和
	double SumSij = 0;
	double Sum_Sij_rjx = 0;

	for (int i = 0; i < NumOfUser; i++)//外层遍历用户
	{
		int j = BinSearch(Data, i, MovieNum[i], ItemID);

		if (j != -1)//如果该用户也看过目标电影
		{
			double a = PearSimAB(UserID, i);
			if (a > 0) //如果两用户是相似用户
			{
				SumSij += a;
				Sum_Sij_rjx += a * (Data[i][j].ScoreOfMovie - bxi(i, Data[i][j].IDOfMovie, u));
			}
		}
	}

	if (SumSij != 0 && Sum_Sij_rjx != 0)
	{
		double rix = Sum_Sij_rjx / SumSij;
		return rix;
	}

	else
	{
		double rix = 1000;
		return rix;
	}

}

//最终产品的终极得分
double End(int UserID, int ItemID, double u)
{
	double End = 0;
	double rxi1 = rxi(UserID, ItemID, u);

	if (rxi1 == 1000) //如果目标用户的相似用户都没有看过目标电影
	{
		//以目标用户打分平均值作为结果
		End = AvgUser(UserID);
		return End;
	}
	else
	{
		End = rxi1 + bxi(UserID, ItemID, u);
		if (End > 100)
			End = 100;
		if (End < 0)
			End = 0;
		return End;
	}
}

//把需要测试的数据（test.txt）读入
void ReadtoBuffer()
{
	ifstream infile;
	infile.open("test.txt");
	assert(infile.is_open());

	string s; //存放读取行数据
	string blank = " ";
	string line = "|";
	int UserID = -1; //user递增，可以直接先赋值为-1
	int Row = -1;
	int i = 0;

	while (getline(infile, s))
	{
		int LocLine = s.find(line); //查找该行中"|"所在位置
		if (s == "")
			break;
		if (LocLine != -1)
		{
			Row++;
			i = 0;
		}
		else
		{
			ReadBuffer[Row][i] = stoi(s);
			i++;
		}
	}
	infile.close();
}

//把读入的数据进行测试，并写入结果文档（text.txt）
void WriteText(double u)
{
	fstream file1;
	file1.open("Text.txt");
	for (int i = 0; i < 19835; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (!j)
			{
				file1 << i << "|" << "6" << endl;
				file1 << ReadBuffer[i][j] << "  " << int(End(i, ReadBuffer[i][j], u)) << endl;
			}
			else
				file1 << ReadBuffer[i][j] << "  " << int(End(i, ReadBuffer[i][j], u)) << endl;
		}
	}
	file1.close();
}


int main()
{
	/*—————必须运行—————*/
	HANDLE hThread;
	DWORD ThreadID;
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadtoBuffer, NULL, 0, &ThreadID);
	//创建一个线程，在load()运行的同时同时把test.txt的数据读入

	load();
	double u = AvgAll();
	SortMovieID();
	Pearson();
	WriteText(u);
	/*——————————————*/

	return 0;
}