// MyPlugin.cpp : 通达信行情软件插件选股代码示例，根据通达信官方模板改编。

#include "stdafx.h"
#undef min
#undef max
#include <stdio.h>
#include <cmath>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <boost/config/warning_disable.hpp>
#include <boost/algorithm/string.hpp>
//#include <fstream>
//#include <iostream>
#include "log.h"
#include "Plugin.h"

//#define PLUGIN_EXPORTS

//static std::pair<int, char const*> DataTypes_[] = {
//	{ PER_MIN5, "5分钟数据" }
//	, { PER_MIN15, "15分钟数据" }
//	, { PER_MIN30, "30分钟数据" }
//	, { PER_HOUR, "1小时数据" }
//	, { PER_DAY, "日线数据" }
//	, { PER_WEEK, "周线数据" }
//	, { PER_MONTH, "月线数据" }
//	, { PER_MIN1, "1分钟数据" }
//	, { PER_MINN, "多分析数据(10)" }
//	, { PER_DAYN, "多天线数据(45)" }
//	, { PER_SEASON, "季线数据" }
//	, { PER_YEAR, "年线数据" }
//	, { PER_SEC5, "5秒线" }
//	, { PER_SECN, "多秒线(15)" }
//	, { PER_PRD_DIY0, "DIY周期" }
//	, { PER_PRD_DIY10, "DIY周期" }
//	, { REPORT_DAT2, "行情数据(第二版)" }
//	, { GBINFO_DAT, "股本信息" }
//	, { STKINFO_DAT, "股票相关数据 STOCKINFO" }
//	, { TPPRICE_DAT, "涨跌停数据" }
//};

struct GDef {
	static GDef& ref();

	template <typename ...T> static long tdx_read(T&&... a) { return (*ref().tdx_read_)(a...); }

	~GDef() {
		fclose(logging::logfile());
	}

	PDATAIOFUNC	 tdx_read_;

	typedef BOOL(*FuncType)(char* Code, short nSetCode
		, int Value[4]
		, short DataType, NTime time1, NTime time2, BYTE nTQ, unsigned long unused);
	FuncType funcs_[8];
	
private:
	static BOOL func0(char* Code, short nSetCode
		, int Value[4]
		, short DataType, NTime time1, NTime time2, BYTE nTQ, unsigned long unused) {
			//LOG << Code << "Type:" << DataType << (int)nTQ << time1 << time2 << boost::make_iterator_range(&Value[0], &Value[4]);
	}
	GDef();
};

//本函数DllMain供调用此DLL的应用程序使用，不可更改，必须保留。
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	GDef::ref();
	LOG << hModule << ul_reason_for_call << lpReserved;

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
    }

    return TRUE;
}

//将回调函数入口赋值给g_pFuncCallBack，自编程序中可以就可以使用g_pFuncCallBack函数调用通达信内部数据（如开盘价、收盘价、最高价、最低价、成交金额、成交量等等（具体数据结构参见OutStruct.h）。

PLUGIN_API void RegisterDataInterface(PDATAIOFUNC pfn)
{
	LOG << pfn;
	GDef::ref().tdx_read_ = pfn;
}

//注: 本文件以上部分需要完整保留，不要做任何修改（除非你知道自己在做什么）

///////////////////// 注册插件信息 ///////////////////////
//注册插件信息:将公式名称，公式描述，参数数量、参数名字、参数最大最小及默认值等信息传送给通达信，并将存储区地址传递给LPPUGIN结构的指针info（具体数据结构将Plugin.h。本函数的具体参数赋值可以根据用户需要修改。
PLUGIN_API void GetCopyRightInfo(PluginInfo* info)
{
	//填写基本信息
	strcpy(info->Name,"Find-1"); //此信息会显示在通达信插件选股对话框中“名称”栏
	strcpy(info->Dy,"Shang_Hai");
	strcpy(info->Author,"oops");	//此信息会显示在通达信插件选股对话框中“设计人”栏
	strcpy(info->Period,"Short-Line");		//还不清楚有什么用
	strcpy(info->Descript,"MA-UP-CROSS");	//此信息会显示在通达信插件选股对话框中“选股对象”栏
	strcpy(info->OtherInfo, "Nothing.");  //strcpy(info->OtherInfo, "无其它信息描述");
	//填写参数信息
	info->ParamNum = 4;	//定义插件参数数量，必须是[0~4]之间整数；

	strcpy(info->ParamInfo[0].acParaName, "Func"); // strcpy(info->ParamInfo[0].acParaName, "MA短天数"); //第一个参数的名称
	info->ParamInfo[0].nMin=0;	//这部分通达信官方模板错误写成nMax，应该为nMin，参数最小值；
	info->ParamInfo[0].nMax=100;	//定义参数最大值
	info->ParamInfo[0].nDefault=0;	//定义参数默认值
	
	strcpy(info->ParamInfo[1].acParaName, "arg1"); // strcpy(info->ParamInfo[1].acParaName, "MA长天数"); //第二个参数的名称
	info->ParamInfo[1].nMin=0;
	info->ParamInfo[1].nMax=100;
	info->ParamInfo[1].nDefault=0;

	strcpy(info->ParamInfo[2].acParaName, "arg2"); // strcpy(info->ParamInfo[1].acParaName, "MA长天数"); //第二个参数的名称
	info->ParamInfo[2].nMin=0;
	info->ParamInfo[2].nMax=100;
	info->ParamInfo[2].nDefault=0;

	strcpy(info->ParamInfo[3].acParaName, "arg3"); // strcpy(info->ParamInfo[1].acParaName, "MA长天数"); //第二个参数的名称
	info->ParamInfo[3].nMin=0;
	info->ParamInfo[3].nMax=100;
	info->ParamInfo[3].nDefault=0;
}

////////////////////////////////用户自定义选股公式部分/////////////////////////////////////////
//自定义选股公式实现细节函数(可根据选股需要添加)

static const	BYTE	g_nAvoidMask[]={0xF8,0xF8,0xF8,0xF8};	// 无效数据标志(通达信系统定义)

static WORD   AfxRightData(float *pData,WORD nMaxData)	//获取有效数据位置
{	
	for (WORD nIndex=0; nIndex<nMaxData; nIndex++) {
        if (memcmp(&pData[nIndex],g_nAvoidMask,4) == 0)
            return nIndex;
    }
	return(nMaxData);
}

//计算简单移动平均MA,通达信模板原函数未做任何修改
static void   AfxCalcMa(float*pData,long nData, WORD nParam) 
{	
	if(pData==NULL||nData==0||nParam==1)
        return;

	long i=nData-nParam+1, nMinEx=AfxRightData(pData,nData);

	if(nParam==0||nParam+nMinEx>nData) {
        nMinEx=nData;
    } else {	
		float nDataEx=0,nDataSave=0;
		float *MaPtr=pData+nData-1,*DataPtr=pData+nData-nParam;
		for (nMinEx+=nParam-1;i<nData;nDataEx+=pData[i++])
            ;
		for (i=nData-1;i>=nMinEx;i--,MaPtr--,DataPtr--) {
			nDataEx+=(*DataPtr);
			nDataSave=(*MaPtr);
			*MaPtr=nDataEx/nParam;
			nDataEx-=nDataSave;
		}
	}
}

//判断穿越，返回值0：当前未发生穿越；1：上穿；2：下穿
static WORD   AfxCross(float*psData,float*plData,WORD nIndex,float* nCross)
{	
	if(psData==NULL||plData==NULL||nIndex==0) return(0);
	float  nDif=psData[nIndex-1]-plData[nIndex-1];
	float  nDifEx=plData[nIndex]-psData[nIndex];
	float  nRatio=(nDif+nDifEx)?nDif/(nDif+nDifEx):0;
	*nCross=psData[nIndex-1]+(psData[nIndex]-psData[nIndex-1])*nRatio;
	if(nDif<0&&nDifEx<0)	return(1);
	if(nDif>0&&nDifEx>0)	return(2);
	return(0);
}

static void print_his(char* Code, short nSetCode, short DataType, short nDataNum, BYTE nTQ)
{
	NTime tmpTime = { 0 };
	std::vector<HISDAT> vec(nDataNum);

	int n = GDef::tdx_read(Code, nSetCode, PER_MIN5, &vec[0], nDataNum, tmpTime, tmpTime, nTQ, 0);  //利用回调函数申请数据，返回得到的数据个数
	LOG << Code << DataType << nDataNum << ":" << n;
	vec.resize(n > 0 ? n : 0);
	for (auto& x : vec)
		LOG << x;
}

//当按照时间段选股时会自动调用此函数
PLUGIN_API BOOL InputInfoThenCalc2(char* Code, short nSetCode
        , int Value[4]
		, short DataType, NTime t0, NTime t1, BYTE nTQ, unsigned long nDataNum)  //选取区段
{
	if (t0 != NTime{ 0 })
		nDataNum = 8;
	BOOL r = (*GDef::ref().funcs_[Value[0]]) (Code, nSetCode, Value, DataType, t0, t1, nTQ, nDataNum);
	if (r) {
		LOG << Code << nSetCode << "Type:" << DataType << (int)nTQ << t0 << t1 << boost::make_iterator_range(&Value[0], &Value[4]);
	}
	return r;
}

//用通达信本地保存的所有数据进行选股，函数原型必须保持不变，系统会自动将需要的参数传递到本函数名字空间内
//Code:股票代码；
//nSetCode:所属市场（0为深市，1为沪市）；
//Value存参数值（通达信插件选股时用户设定值）
//DataType:由通达信行情软件传递过来的数据，4为日线，5为周线，参见OutStruct.h中宏定义。
//    默认为日线数据。如申请行情数据则赋值为REPORT_DAT2，其他相关类型参见OutStruct.h
//nDataNum为申请数据个数，
//    红宝书上讲，若为-1且pData为NULL则函数返回历史数据个数，
//    然而取到的数据必须存储到pData中，参数pData不允许为NULL
//    所以以上说法还需要继续研究，本程序中取nDataNum=2000
//nTQ:复权模式，0：不复权；1：前复权

PLUGIN_API BOOL InputInfoThenCalc1(char* Code, short nSetCode
		, int Value[4]
		, short DataType, short nDataNum
        , BYTE nTQ, unsigned long unused) //按最近数据计算
{
	NTime t0 = { 0 };
	return InputInfoThenCalc2(Code, nSetCode, Value, DataType, t0, t0, nTQ, nDataNum);

	//LOG << Code << "Type:" << DataType << nDataNum << (int)nTQ << boost::make_iterator_range(&Value[0], &Value[4]);

	//STOCKINFO info = {};
	//REPORTDAT2 hq = {};
	//NTime tmpTime = { 0 };
	//	int n;

	//n = GDef::tdx_read(Code, nSetCode, STKINFO_DAT, &info, 1, tmpTime, tmpTime, nTQ, 0);
	//LOG << Code << " STKINFO_DAT:" << n << "\n" << info;

	//n = GDef::tdx_read(Code, nSetCode, REPORT_DAT2, &hq, 1, tmpTime, tmpTime, nTQ, 0);
	//LOG << Code << " REPORT_DAT2:" << n << "\n" << hq;

	//int ac = atoi(Code);
	//if (ac == 2405 || ac == 2241) {
	//	std::vector<HISDAT> his(std::min(int(nDataNum), 8));

	//	n = GDef::tdx_read(Code, nSetCode, PER_DAY, &his[0], his.size(), tmpTime, tmpTime, nTQ, 0);
	//	his.resize(n > 0 ? n : 0);
	//	LOG << Code << DataType << nDataNum << ":" << n;

	//	for (auto& x : his) {
	//		LOG << x
	//			<< "rate-ex" << std::fixed << std::setprecision(2) << x.fVolume / (info.ActiveCapital/100);
	//	}

	//	print_his(Code, nSetCode, PER_MIN1, 4, nTQ);
	//	print_his(Code, nSetCode, PER_MIN15, 4, nTQ);
	//	return true;
	//}
	
	//if (readnum > std::max(Value[0], Value[1])) //只有数据量（个数）大于Value[0]和Value[1]中的最大值才有意义，否则算不出对应周期的MA结果。

	//return FALSE; "D:/Program_Files/new_tdx/T0002/blocknew/ZXG.blk";
	
	//{
    //    std::vector<float> Ma1(readnum);
    //    std::vector<float> Ma2(readnum);

	//	for (int i=0; i < readnum; i++)
	//	{
	//		Ma1[i] = HisDat[i].Close;
	//		Ma2[i] = HisDat[i].Close;
	//	}

	//	AfxCalcMa(&Ma1[0],readnum,Value[0]);	//计算MA
	//	AfxCalcMa(&Ma2[0],readnum,Value[1]);
	//	float nCross;

	//	if (AfxCross(&Ma1[0],&Ma2[0],readnum-1, &nCross) == 1)//判断是不是在readnum-1(最后一个数据)处交叉 1:上穿 2:下穿
	//	{
	//		nRet = TRUE;  //返回为真，符合选股条件
	//		FILE *fp;
	//		//if((fp = fopen("D:\\tdx-plugin-选股.txt","a"))==NULL)
	//			fp = fopen("D:\\tdx-plugin-find.txt","w");
	//		fprintf(fp, Code);
	//		fprintf(fp, ";0;10000;Buy;0.2\n");
	//		fclose(fp);
	//	}
	//	//delete []pMa1;pMa1=NULL;
	//	//delete []pMa2;pMa2=NULL;
	//}

	//delete []pHisDat;pHisDat=NULL;
	//return nRet;
}


#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/locale/encoding_utf.hpp>
#include <fstream>

namespace dfcf
{
	static char const* const FN_ZXG = "D:\\Program_Files\\eastmoney\\swc8\\config\\User\\m8124094360778600\\StockwayStock.ini";

	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;
	namespace phoenix = boost::phoenix;

	template <typename Iterator, typename Set>
	bool parse_numbers(Iterator begin, Iterator end, Set& v)
	{
		using qi::int_;
		using qi::char_;
		using qi::lit;
		using qi::_1;
		using phoenix::insert;
		bool r = qi::phrase_parse(begin, end
			, ('=' >> ((char_ >> '.' >> int_[insert(phoenix::ref(v), _1)]) % ',') >> *lit(','))
			, ascii::space, v);
		return (begin == end && r);
	}

	std::set<int> get_ZXG(char const* fn = FN_ZXG)
	{
		static const char kZXG[] = { '\xea', '\x81', '\x09', '\x90', '\xa1', '\x80' }; //UTF-16 "自选股"

		std::string line;
		std::ifstream fp(fn, std::ios::binary);
		while (getline(fp, line)) {
			const char *it, *end;

			end = &line[line.size()];
			it = std::search(line.c_str(), end, &kZXG[0], &kZXG[sizeof(kZXG)]);
			if (it == end) {
				//std::cerr << line.size() <<"\t[Not-Found]\n";
				continue;
			}
			it += sizeof(kZXG);
			if ((end - it) & 1) {
				//std::cerr << line.size() <<"\t"<< "\t[Length-Error]\n";
				continue;
			}

			std::string utf8 = boost::locale::conv::utf_to_utf<char>
				(reinterpret_cast<const char16_t*>(it), reinterpret_cast<const char16_t*>(end));
			//std::cerr << utf8 <<"\t[]\n";
			it = &utf8[0];
			end = &utf8[utf8.size()];

			if (*it != '=') // TODO
				continue;
			//it = std::find(it, end, '=');

			std::set<int> v;
			parse_numbers(it, end, v);
			return std::move(v);
		}
		return std::set<int>();
	}

	static BOOL read_dfcf_ZXG(char* Code, short nSetCode
		, int Value[4]
		, short DataType, NTime time1, NTime time2, BYTE nTQ, unsigned long unused)  //选取区段
	{
		LOG << Code << "Type:" << DataType << (int)nTQ << time1 << time2 << boost::make_iterator_range(&Value[0], &Value[4]);
		static std::set<int> zxg;
		if (zxg.empty()) {
			zxg = dfcf::get_ZXG();
			zxg.insert(0);
		}
		if (zxg.find(atoi(Code)) != zxg.end()) {
			return TRUE;
		}
		return FALSE;
	}
}

static BOOL Pc_o(char* Code, short nSetCode
	, int Value[4]
	, short DataType, NTime t0, NTime t1, BYTE nTQ, unsigned long)  //选取区段
{
	int n;
	REPORTDAT2 hq = {};
	n = GDef::tdx_read(Code, nSetCode, REPORT_DAT2, &hq, 1, t0, t1, nTQ, 0);
	if (n < 1 || hq.Volume*100 < 1) {
		return FALSE;
	}
	// if (hq.Now < hq.Close) { return FALSE; }
	float a = (hq.Now - hq.Close) / hq.Close;
	// if (a < 0) { return FALSE; }

	float minval = float(Value[1]/1000.0);
	if (-0.001 < minval && minval < 0.001)
		minval = -0.0085f;
	float maxval = float(Value[2]/1000.0);
	if (-0.001 < maxval && maxval < 0.001)
		maxval = 0.04f;

	if (a < minval || a > maxval) {
		return FALSE;
	}

	LOG << hq <<"#"<< a <<"%";
	return TRUE;
}

static BOOL myflt0(char* Code, short nSetCode
	, int Value[4]
	, short DataType, NTime t0, NTime t1, BYTE nTQ, unsigned long)  //选取区段
{
	int n;

	REPORTDAT2 smx = {};
	n = GDef::tdx_read(Code, nSetCode, REPORT_DAT2, &smx, 1, t0, t1, nTQ, 0);
	if (n < 1 || smx.Volume*100 < 1 || (smx.Now - smx.Close)/smx.Close < -0.008) {
		LOG << Code << "Ig" << "Vol" << smx.Volume <<"Price"<< smx.Now << smx.Close;
		return FALSE;
	}
	STOCKINFO sinf = {};
	n = GDef::tdx_read(Code, nSetCode, STKINFO_DAT, &sinf, 1, t0, t1, nTQ, 0);
	//if (smx.Volume*100 / sinf.ActiveCapital < 0.04) { return FALSE; }

	std::vector<HISDAT> his(6); // typedef std::vector<HISDAT>::reverse_iterator itertype;
	n = GDef::tdx_read(Code, nSetCode, PER_DAY, &his[0], his.size(), t0, t1, nTQ, 0);
	if (n < (int)his.size()) {
		LOG << Code << "Few"<< n;
		return FALSE;
	}
	// his.resize(n);

	auto rbeg = his.rbegin();
	auto rnext = rbeg + 1;
	if (rbeg->Close < rbeg->Open && (rbeg->Close - rnext->Close)/rnext->Close < -0.01) {
		return FALSE;
	}

	auto lowp = std::min_element(his.rbegin(), his.rend()
		, [](HISDAT const& lhs, HISDAT const& rhs){
			return std::min(lhs.Open, lhs.Close) < std::min(rhs.Open,rhs.Close);
		});
	if (lowp  > rbeg + 3) {
		return FALSE;
	}
	//auto it1= std::max_element(his.rbegin(), lowp +1
	//	, [](HISDAT const& lhs, HISDAT const& rhs){
	//		return std::max(lhs.Open, lhs.Close) < std::max(rhs.Open,rhs.Close);
	//	});
	//if (it1->Close < it1->Open) {
	//	return FALSE;
	//}
	//if ((it1->Close - lowp ->Open)/lowp ->Open < 0.06) {
	//	return TRUE;
	//}
	float upr = (rbeg->Close - lowp->Close) / lowp->Close;
	if (lowp == rbeg) {
		upr = (rbeg->Close - rnext->Close) / rnext->Close;
	}
	if (upr < -0.0088 || upr > 0.15) {
		return FALSE;
	}
	float exr = smx.Volume*100.0 / sinf.ActiveCapital;
	if (exr < 0.04 || exr > 0.1) {
	LOG <<Code << "Volume" << int(rbeg->fVolume) << smx.Volume << smx.NowVol << int(sinf.ActiveCapital);
		return FALSE;
	}
	if ((rbeg->fVolume - rnext->fVolume) / sinf.ActiveCapital < 0.009) {
		return FALSE;
	}
	//if ((rbeg->fVolume - lowp->fVolume) * 100 / sinf.ActiveCapital < (lowp - rbeg) * 0.004 + 0.0025) { return FALSE; }
	return TRUE;
}
static BOOL myflt1(char* Code, short nSetCode
	, int Value[4]
	, short DataType, NTime t0, NTime t1, BYTE nTQ, unsigned long)  //选取区段
{
	int n;

	REPORTDAT2 smx = {};
	n = GDef::tdx_read(Code, nSetCode, REPORT_DAT2, &smx, 1, t0, t1, nTQ, 0);
	if (n < 1 || smx.Volume*100 < 1) {
		LOG << Code << "Ig" << "Vol" << smx.Volume <<"Price"<< smx.Now << smx.Close;
		return FALSE;
	}

	STOCKINFO sinf = {};
	n = GDef::tdx_read(Code, nSetCode, STKINFO_DAT, &sinf, 1, t0, t1, nTQ, 0);
	if (n<0 || smx.Volume*100 / sinf.ActiveCapital < 0.05) {
		return FALSE;
	}

	std::vector<HISDAT> his(6); // typedef std::vector<HISDAT>::reverse_iterator itertype;
	n = GDef::tdx_read(Code, nSetCode, PER_DAY, &his[0], his.size(), t0, t1, nTQ, 0);
	if (n < (int)his.size()) {
		LOG << Code << "Few"<< n;
		return FALSE;
	}
	// his.resize(n);
	auto rbeg = his.rbegin();
	auto rnext = rbeg + 1;

	if ((rbeg->Close - rnext->Close) / rnext->Close < 0.04) {
		return FALSE;
	}

	if ((rbeg->fVolume - rnext->fVolume) / sinf.ActiveCapital < 0.015) {
		return FALSE;
	}

	LOG << Code <<"("<< int(rbeg->fVolume) <<"-"<<int(rnext->fVolume) <<")/"<< int(sinf.ActiveCapital/100)
		<< rbeg->Time;

	return TRUE;
}

static BOOL prev_flt(char* Code, short nSetCode
	, int Value[4]
	, short DataType, NTime t0, NTime t1, BYTE nTQ, unsigned long )  //选取区段
{
	// int ac = atoi(Code); // if (ac != 600570) return FALSE; // Test

	std::vector<HISDAT> his(8); //(std::max(int(nData),8)); // (std::min(int(nDataNum), 8));
	int n;

	n = GDef::tdx_read(Code, nSetCode, PER_DAY, &his[0], his.size(), t0, t1, nTQ, 0);
	if (n < 1) { return FALSE; }
	if (n == 1) {
		LOG << his[0] << "only 1";
		return TRUE;
	}
	if (his[n-1].fVolume < 1) {
		LOG << Code << "TP/Stop";
		return FALSE;
	}
	his.resize(n);

	STOCKINFO brief = {};
	n = GDef::tdx_read(Code, nSetCode, STKINFO_DAT, &brief, 1, t0, t1, nTQ, 0);

	auto rbeg = his.rbegin();
	auto rnext = rbeg+1;
	if (rbeg->fVolume < rnext->fVolume
		|| (rbeg->fVolume - rnext->fVolume)*100 / brief.ActiveCapital < std::max(Value[1],10)/1000) {
		return FALSE;
	}
	if (rbeg->Close < rnext->Close
		|| (rbeg->Close - rnext->Close) / rnext->Close < std::max(Value[2],25)/1000) {
		return FALSE;
	}

	REPORTDAT2 hq = {};
	n = GDef::tdx_read(Code, nSetCode, REPORT_DAT2, &hq, 1, t0, t1, nTQ, 0);

	for (auto& x : his) {
		LOG << x << "rate-ex" << std::fixed << std::setprecision(2)
				<< (x.fVolume*100 / brief.ActiveCapital) << (hq.Volume*100 / brief.ActiveCapital)
			;
	}
	LOG << Code << " STKINFO_DAT:" << n << "\n" << brief;
	LOG << Code << " REPORT_DAT2:" << n << "\n" << hq;
	LOG << Code << DataType << his.size() << ":" << n;

	print_his(Code, nSetCode, PER_MIN1, 4, nTQ);
	print_his(Code, nSetCode, PER_MIN15, 4, nTQ);
	return TRUE;

	//if (readnum > std::max(Value[0], Value[1])) //只有数据量（个数）大于Value[0]和Value[1]中的最大值才有意义，否则算不出对应周期的MA结果。

	return FALSE; "D:/Program_Files/new_tdx/T0002/blocknew/ZXG.blk";
}

static BOOL defaf(char* Code, short nSetCode
	, int Value[4]
	, short DataType, NTime time1, NTime time2, BYTE nTQ, unsigned long unused)  //选取区段
{
	return atoi(Code) == 600570;
}

GDef::GDef()  {
	logging::logfile( fopen("D:/home/wood/stock/tdx.log", "w") );
	for (auto& f : funcs_)
		f = defaf;
	funcs_[0] = myflt0; // Default
	funcs_[1] = myflt1;
	funcs_[2] = prev_flt;
	funcs_[7] = dfcf::read_dfcf_ZXG;
}

GDef& GDef::ref() {
	static GDef def;
	return def;
}

	//LOG << Code << nSetCode << Value <<"Type:"<< DataType << time1 << time2 << nTQ;
	//return false;

	//BOOL nRet = FALSE;
	//NTime tmpTime={0};

	////窥视数据个数
	//long datanum = GDef::tdx_read(Code,nSetCode,DataType,NULL,-1,time1,time2,nTQ,0);
	//if( datanum < std::max(Value[0],Value[1]) ) 
	//	return FALSE;
	//
	////读取数据
	//LPHISDAT pHisDat = new HISDAT[datanum];
	//long readnum = GDef::tdx_read(Code,nSetCode,DataType,pHisDat,datanum,time1,time2,nTQ,0);
	//if( readnum > std::max(Value[0],Value[1]) ) //只有将数据个数大于Value[0]和Value[1]中的最大值才有意义
	//{
	//	float *pMa1 = new float[readnum];
	//	float *pMa2 = new float[readnum];
	//	for(int i=0;i < readnum;i++)
	//	{
	//		pMa1[i] = pHisDat[i].Close;
	//		pMa2[i] = pHisDat[i].Close;
	//	}
	//	AfxCalcMa(pMa1,readnum,Value[0]);	//计算MA
	//	AfxCalcMa(pMa2,readnum,Value[1]);
	//	float nCross;
	//	if(AfxCross(pMa1,pMa2,readnum-1, &nCross) == 1)	//判断是不是在readnum-1(最后一个数据)处交叉 1:上穿 2:下穿
	//	{
	//		nRet = TRUE;  //返回为真，符合选股条件
	//		FILE *fp;
	//		if((fp = fopen("D:\\Tdx-plugin-find.txt","a"))==NULL)
	//		fp = fopen("D:\\Tdx-plugin-find.txt","w");
	//		fprintf(fp, Code);
	//		fprintf(fp, ";0;10000;Buy;0.2\n");
	//		fclose(fp);
	//	}

	//	delete []pMa1;pMa1=NULL;
	//	delete []pMa2;pMa2=NULL;
	//}

	//delete []pHisDat;pHisDat=NULL;
	//return nRet;