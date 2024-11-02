#include "resource.h"
#include "BForm.h"
#include "BHashStrK.h"
#include "mdlOpenSaveDlg.h"
#include "BADO.h"
#include "BDijkstra.h"
#include "BReadLinesEx.h"

CBForm form1(ID_form1);
//再声明另一CPP文件中关于显示结果预览界面的函数，若需在此cpp文件中使用该函数
//则需要使用extern +变量定义来声明，变量、对象、函数都需此操作。
extern CBForm Form_FrmView;																
extern void EventsMapFrmView();															//加载结果预览界面的各控件与其功能									
extern bool BREAK_SIG;

bool IfCancel_Sign = false;																//用于标记用户是否申请取消当前操作。

//程序预处理↓
void Preset_Welcome()
{
	form1.Control(IDC_Static_Doing).TextSet(TEXT("程序待启动，请按照先后顺序启动程序：\n1.载入PPI数据文件并提取相应的物种ID到数据库。\n2.载入ENSP数据集文件\n3.点击分析按钮。"));
																						//Static_Doing显示当前状态，并提醒用户进行操作。
	form1.Control(IDC_Static_Status).TextSet(TEXT(""));					//清空Static_Status文本
	form1.Control(IDC_Static_Count).TextSet(TEXT(""));					//清空Static_Count文本
	//MsgBox(TEXT("该程序旨在：尝试分析蛋白间的作用关系，若出现问题请联系：\nQQ:1537361071\n邮箱:1537361071@qq.com"), TEXT("欢迎！"), mb_OK, mb_IconNone);
		
	if (MsgBox(TEXT("该程序旨在：尝试分析蛋白间的作用关系，若出现问题请联系：\nQQ:1537361071\n邮箱:1537361071@qq.com"), TEXT("欢迎！"), mb_YesNoCancel, mb_IconNone) != idYes)
	{
		return;
	}
	//弹出迎接用户对话框。
	return;
}
//分析结果预览按钮↓
void Button_PreViewResult()
{
	Form_FrmView.Show();																//显示分析结果预览窗口。
}
/*PPI读取函数，从Start_Position位置开始读取数据内容，一直读到数据开头不为String_Find的行，同时对当前行入数据
若读取写入成功，则返回操作完成的数据量，若读取写入失败或无数据可读取写入，则返回0↓*/
long ReadPPIToDB(LPCTSTR Size_File, LONGLONG Start_Position, LPCTSTR String_Find)
{
	CBAdoRecordset rs;																	//声明新的CBAdoRecordeset结构体来进行下述对ACESS数据库的操作。
	if (!rs.Open(TEXT("SELECT * FROM ProtLinks")))									//若未成功对目标ACESS数据库进行写入，则中止程序并报错。
	{
		MsgBox(TEXT("对Acess数据库中的ProtLinks写入失败！"), TEXT("Acess写入失败"), mb_OK, mb_IconExclamation);
		return 0;
	}
	if (!rs.EOFRs())																	//EOF()返回此时是否处于最后一行，若处于则EOF=1，否则EOF=0。
	{
		if (MsgBox(TEXT("数据库中的 ProtLinks 表中已有记录。要向其中添加新纪录，必须删除原有记录。\r\n是否删除原有记录？"), TEXT("清空数据库表"), mb_YesNoCancel, mb_IconExclamation) != idYes)										
			return 0;																	//询问是否要从头开始记录分析样本。
		pApp->MousePointerGlobalSet(IDC_Wait);										//指针变为等待，以提醒用户此时程序正在进行删除操作
		form1.Control(IDC_Static_Status).TextSet(TEXT("正在删除 ProtLinks 表中已有记录……"));
																						//显示即将进行清空ProtLinks表格操作
		ADOConn.Execute(TEXT("DELETE FROM ProtLinks"));							//在Acess中输入SQL语句以清空ProtLinks表格
		form1.Control(IDC_Static_Status).TextSet(TEXT("已清空原数据记录"));
																						//显示清空顺利完成
		pApp->MousePointerGlobalSet(0);										//指针变回常规，表示清空操作顺利完成。	
	}

	char * Aiming_Find = StrConvFromUnicode(String_Find);						//开新的指针指向要查找的变量
	long Length_Find = strlen(Aiming_Find);											//目标为ANSI格式，所以用strlen()函数来取要查找的内容长度	
	CBReadLinesEx Aiming_File;															//
	tstring String_Field;																//用于判断某个字段的内容
	long Imported_Count = 0;															//记录已导入的数据数量
	LPTSTR Size_Line;																	//
	TCHAR ** s;
	int n;
	CBControl Static_Status(IDC_Static_Status);											//用CBControl类来简要定义Static_Status块，方便后续操作
	if (!Aiming_File.OpenFile(Size_File))										//打开目标文件，若未成功打开则直接退出
		return 0;	
	Aiming_File.SeekFile(Start_Position);											//从已记录的起始点开始读取文件
	while (!Aiming_File.IsEndRead())													//若读取到最后一行则停止循环
	{
		Aiming_File.GetNextLine(Size_Line);										//移动到下一行数据
		if (Aiming_File.IsErrOccured())													//若改行数据无法获取，则退出循环
			return 0;
		n = Split(Size_Line, s, TEXT(" "));					//将该行数据内容按照空格分隔并储存在s数据组中，从s[1]开始依次储存，并将该行分隔后的数据数量返回到变量n中
		if (n < 3)																		//每行应有三行数据，若该行数据小于3则该行无效，不做处理，直接进入下一层循环
			continue;
		String_Field = s[1];															//借助其那面定义的String_Field变量来判断所读取数据是否合法
		if (String_Field.substr(0, Length_Find) != String_Find)				//若读取的不是“9606.ENSP”数据文件则终止循环
			break;
		rs.AddNew();																	//开始新增数据
		rs.SetField(TEXT("ENSPID1"), (int)Val(s[1] + Length_Find));
																						//定位到9606.ENSP之后的数字，将s[1]这部分数字以整形的方式添加到ENSPID1列的当前行
		rs.SetField(TEXT("ENSPID2"), (int)Val(s[2] + Length_Find));
																						//定位到9606.ENSP之后的数字，将s[1]这部分数字以整形的方式添加到ENSPID1列的当前行
		rs.SetField(TEXT("Distance"), 1000 - (int)Val(s[3])); 
		//前面已经将计算好的蛋白作用关系的值，储存到了s[3]当中，关联性越强，则s[3]越大，且s[3]∈[0,1000],
		//本项目中所包含的默认Dijkstra模块作用是求结点间的最短路径，无法求最长路径，所以用1000-两个蛋白之间的s[3],
		//来代表两个蛋白结点间的距离，此时距离越短代表这两个蛋白关联性越强，这样，通过Dijkstra算法求得任意两个蛋白
		//结点间的最短路径，即为这两个蛋白结点之间的最短距离，这个值越小代表关联性越强。
		
		rs.Update();																	//确认上述SetField改动
		Imported_Count++;																//已完成的数据量+1
		if (Imported_Count % 300 == 0)													//每300条数据更新，刷新一次进度显示文本
		{
			Static_Status.TextSet(TEXT("已导入: "));
			Static_Status.TextAdd(Imported_Count);
			Static_Status.TextAdd(TEXT(" 条记录\r\n"));
			Static_Status.TextAdd(Size_Line);
			DoEvents();																	//让出CPU
			if (IfCancel_Sign)															//检查用户是否点击中断，若用户选择停止，则停止分析
			{
				Static_Status.TextSet(TEXT("用户中断。"));
				break;
			}
		}
	}
	rs.Close();																			//关闭记录集，结束并保存当前部分操作
	if (!IfCancel_Sign)																	//若用户未选择中断，则显示已完成的工作
	{
		Static_Status.TextSet(TEXT("数据导入完成，共导入 "));
		Static_Status.TextAdd(Imported_Count);
		Static_Status.TextAdd(TEXT(" 条记录"));
	}
	return Imported_Count;																//返回已更新的数据量。
}
//定位目标内容（9606.ENSP）位置的函数↓
LONGLONG FindPos_PPIFile(HANDLE Open_Files, LPCTSTR Size_File)
{
	//在Open_Files文件中查找，以字符串Siz_File开头的内容
	char * Aiming_Find = StrConvFromUnicode(Size_File);							//将目标内容转存到Aiming_Find字符串中
	long Length_Finding = strlen(Aiming_Find);										//目标为ANSI格式，所以用strlen()函数来取要查找的内容长度	
	char Buffer[131072];																//缓冲区
	bool If_Found = false;																//标记是否找到目标
	long ReadBytes = 0, ReadCycled = 0, i, k;											
	LONGLONG File_TotalBytes = EFLOF(Open_Files);									//文件的总字节数
	LONGLONG File_Position = 0;															//当前读写到的位置		
	
	while (File_Position < File_TotalBytes)
	{
		ReadBytes = EFGetBytes(Open_Files, File_Position, Buffer, sizeof(Buffer));
			//从File_Position位置开始，不断将Open_Files里的字符存入Buffer数组，直到存满或读取到文件最后一个字节停止。
			//若未成功读取则返回值为负，若成功读取则返回值为此次存入Buffer数组的字符数目。
		if (ReadBytes <= 0)																//若EFGetBytes函数读取文件失败，则返回值小于0，终止循环
			break;
		for (i = 0; i < ReadBytes; i++)
		{
			if (Buffer[i] == 10 || (File_Position == 0 && i == 0))						//找到\n或文件开头的第0个内存位置（即第一个字节）
			{
				if (Buffer[i] == 10)
					i++;																//若buff[i]是\n，则从下一个内存位置开始查找9606.ENSP字符串
				if (ReadBytes - i < Length_Finding)										//ReadBytes-i即剩余字节数目，若剩余字节数目已经不足‘9606.ENSP’字符串所占的字节数目
				{
					File_Position -= (ReadBytes - i);									//则目标指针跳过后面的查找过程，节省了查找时间
					break;																//退出循环，搜索下一段字符串
				}
				for (k = 0; k < Length_Finding; k++)									//寻找满足9606.ENSP的字符串部分
				{
					if (Buffer[i + k] != Aiming_Find[k])								//若有任意一处不满足条件就退出循环进行，节省了计算时间
						break;
				}
				if (k >= Length_Finding)												//如果k>=目标字符串长度，代表目标字符串位置的每一个字节都符合要求，直到下一行才不满足要求，即已成功找到目标位置
				{
					If_Found = true;													//标记已找到，并退出循环
					break;
				}
			}
		}
		if (If_Found)																	//If_Found==1								
			break;																		//若已成功找到目标位置就终止查找过程
		File_Position += ReadBytes;														//目标位置就等于初始位置＋已查询的字节数
		ReadCycled++;																	//循环数+1
		if (ReadCycled % 50 == 0)														//每50次循环输出一次当前查询进度
		{
			form1.Control(IDC_Static_Status).TextSet(TEXT("查找"));		
			form1.Control(IDC_Static_Status).TextAdd((double)(File_Position / 1024 / 1024));
			form1.Control(IDC_Static_Status).TextAdd(TEXT(" MB / "));
			form1.Control(IDC_Static_Status).TextAdd((double)(File_TotalBytes / 1024 / 1024));
			form1.Control(IDC_Static_Status).TextAdd(TEXT(" MB …… "));
			//以上操作为，在静态文本框输出当前的查询进度。
			DoEvents();																	//让出CPU
			if (IfCancel_Sign)															//若用户终止
			{
				form1.Control(IDC_Static_Status).TextSet(TEXT("用户中断读取。"));
				break;																	//显示用户中断读取并退出循环
			}
		}
	}
	if (If_Found)																		//若已找到目标位置
		return File_Position + i;														
			//由于缓存区内存有限，上述程序是通过将文件字节逐一加入Buffer然后查找，完成查找所有Buffer数组位置为1次循环
			//每次循环结束后若没找到目标位置则清空Buffer然后存入文件中后面的字节内容，这样不停循环寻找，直到找到目标内容
			//所以最后返回的值为当前循环的目标位置i加上前面已经走完的所有循环的字节数目。
	else
		return 0;																		//若没找到，则返回0。
}
//浏览本地文件夹并添加PPI数据文件按钮↓
void Button_BrowsPPI()
{
	LPTSTR Size_File;																	//用LPTSTR代存目标文件
	OsdSetFilter(TEXT("文本文件(*.txt)|*.txt|数据文件(*.dat)|*.dat"), true);
																						//寻找的本地文件的格式（即后缀名）
	Size_File = OsdOpenDlg(form1.hWnd(), TEXT("请选择PPI数据文件"));			//选择本地文件的选择框的窗口名。
	if (*Size_File)																		//若已选择有效，且第一个字符为非空的文件	
	{
		form1.Control(IDC_COMB_PPIFiles).AddItem(Size_File);			//将所选择的文件全灵添加到组合框COMB_PPIFiles
		form1.Control(IDC_COMB_PPIFiles).TextSet(Size_File);			//在组合框COMB_PPIFiles中显示所选文件的全路径
	}
	return;
}
//通过鼠标拖动为PPIFiles列表框添加文件函数↓
void COMBPPIFiles_FilesDrop(int ptrArrayFiles, int PPI_Cnt, int x, int y)
{
	TCHAR** Files = (TCHAR**)ptrArrayFiles;
	//将ptrArrFiles转换为TCHAR ** 类型，即二维数组，这样可以使files里的内容保存到二维数组中
	//也就是字符串的一维数组，通过∑file[i]，依次取得file文件中的各个字符串
	//这些字符串就是用户所拖动的文件中，被拖动控件上的各个文件名
	form1.Control(IDC_COMB_PPIFiles).AddItem(Files[1]);
	form1.Control(IDC_COMB_PPIFiles).TextSet(Files[1]);
	//此处只打开一个文件file[1]，将其添加并显示到组合框
	return;
}
/*
为提取到数据库按钮赋予功能：
1.点击“提取到数据库”按钮，提取到数据库，然后按钮变为“取消”
2.点击“取消”按钮时终止提取操作，然后按钮变为“提取到数据库”↓
*/
void Button_ExtractSpeciesID()
{
	//下方if循环为每次函数首先检测是否为“取消状态”，若是，则执行下方循环
	if (_tcscmp(form1.Control(IDC_Button_ExtractSpeciesID).Text(), TEXT("取消")) == 0) 
	{
		if (MsgBox(TEXT("提取尚未完成,确定要取消吗？"), TEXT("取消操作"), mb_YesNoCancel, mb_IconQuestion) != idYes)
			return;
		//点击取消按钮之后，弹出对话框询问用户是否真的要取消，可以防止误触
		//若用户选择“是”，则让全局bool变量IfCancel_Sign变为true，表示用户选择中断提取操作。
		IfCancel_Sign = true;
		return;
	}
	LPTSTR Size_File;
	Size_File = form1.Control(IDC_COMB_PPIFiles).Text();						//用LPTSTR类来存储列表IDC_COMB_PPIFiles的内容
	if (*Size_File == 0)																//若列表为空，则报错并推出
	{
		MsgBox(TEXT("请选择PPI数据文件！"), TEXT("未选择文件"), mb_OK, mb_IconExclamation);
		return;
	}

	int RollsNumber = form1.Control(IDC_COMB_SpeciesIDs).ListIndex();			//取得列表当前行数
	int TaxIDs = form1.Control(IDC_COMB_SpeciesIDs).ItemData(RollsNumber);
																						//取已载入的物种ID
	if (TaxIDs <= 0)																	//若无物种ID则报错并退出
	{
		MsgBox(TEXT("请选择物种ID！"), TEXT("未选择物种ID"), mb_OK, mb_IconExclamation);
		return;
	}
	HANDLE Open_Files;																	
	Open_Files = EFOpen(Size_File);												//如果打开失败则返回值为INVALID_HANDLE_VALUE
	if (Open_Files == INVALID_HANDLE_VALUE)												//若打开失败则报错并退出
	{
		MsgBox(TEXT("打开PPI数据文件失败！"), TEXT("打开文件失败，请验证路径完整性！"), mb_OK, mb_IconExclamation);
		return;
	}
	IfCancel_Sign = false;																//将用户申请取消标志重置为false
	form1.Control(IDC_Button_ExtractSpeciesID).TextSet(TEXT("取消"));	//执行提取功能后将按钮显示改为“取消”

	tstring String_Find;																//保存目标字符串
	String_Find = Str(TaxIDs);							
	String_Find += TEXT(".ENSP");														//拼出“9606.ENSP”
	LONGLONG Aim_Position = FindPos_PPIFile(Open_Files,String_Find.c_str());	//定位到目标StringFind所在的内存位置
																						//FindPos_PPIFile内容部分的解释清查看该函数的定义
	if (Aim_Position == 0)																//返回值为0则代表没找到	
	{
		form1.Control(IDC_Static_Status).TextSet(TEXT("在文件中未找到 : "));
		form1.Control(IDC_Static_Status).TextAdd(String_Find);
																						//提示用户未找到目标
	}
	else
	{
		//若找到目标，则更新状态
		form1.Control(IDC_Static_Status).TextSet(TEXT("在文件中已找到"));
		form1.Control(IDC_Static_Status).TextAdd(String_Find);
		form1.Control(IDC_Static_Status).TextAdd(TEXT(" 的开始位置为 ："));
		form1.Control(IDC_Static_Status).TextAdd((double)Aim_Position);
	}
	form1.Control(IDC_Static_Status).TextAdd(TEXT("。"));			
	EFClose(Open_Files);															//关闭文件
	if (Aim_Position)																	
		ReadPPIToDB(Size_File, Aim_Position, String_Find.c_str());
	//若找到目标，则通过ReadPPIToDB函数，从Aim_Poisiton行开始一只读取到内容不为String_Find的行
	//将读取得到的每行信息存入数据库的ProtLinks表中
	form1.Control(IDC_Button_ExtractSpeciesID).TextSet(TEXT("提取到数据库"));
	//完成提取后，将按钮显示调整回“提取到数据库”
	return;
}
//从目标数据库获得所有的物种ID并显示到IDC_COMB_SpeciesIDs列表框↓
void ShowSpeciesID()
{
	CBAdoRecordset rs;																	//用CBAdoRecordeset类来封装ADO中的RecordSet对象
	tstring Items_Total;																//总的物种ID字符串
	int SpeciesID = 0, RollsNumber = 0;													//初始定义物种ID和列表数目

	if (!rs.Open(TEXT("SELECT * FROM taxes")))										//使用SQL语句在数据库中查找，若访问数据库失败则返回值为0，若成功则返回值为1
	{
		MsgBox(ADOConn.ErrorLastStr(), TEXT("获取物种信息失败！"), mb_OK, mb_IconExclamation);
																						//访问数据库失败，向用户报错
		return;
	}

	form1.Control(IDC_COMB_SpeciesIDs).ListClear();							//清空IDC_COMB_SpeciesIDs列表框的内容

	while (!rs.EOFRs())																	//检测是否读取完，若读取完全部物种ID则终止循环，否则持续循环
	{
		Items_Total = rs.GetField(TEXT("TaxID"));								//取TaxID列的当前行的数据作为第i组物种ID数据
		SpeciesID = (int)Val(Items_Total.c_str());								//将上面刚刚获取的字符串内容转换为整形保存
		Items_Total = TEXT("(") + Items_Total;											//连接两个字符串
		Items_Total = rs.GetField(TEXT("Organism")) + Items_Total + TEXT(")");	//再取Organism列的当前行数据内容加入总字符串。
		RollsNumber = form1.Control(IDC_COMB_SpeciesIDs).AddItem(Items_Total);			//上述转化得到的总字符串即目标物种ID，将其保存到
																						//IDC_COMB_SpeciesIDs列表中，并且把该表当前的已存行数返回到RollsNumber中
		form1.Control(IDC_COMB_SpeciesIDs).ItemDataSet(RollsNumber, SpeciesID);			//在第RollsNumber行显示刚才保存的物种ID
		
		rs.MoveNext();																	//将所读取表格的目标移动到下一行。
	}
	rs.Close();																			//关闭记录集
	if (form1.Control(IDC_COMB_SpeciesIDs).ListCount() > 0)					//若列表不为空，则直接定位到我们需要的HomoSpeicies那一行。
		form1.Control(IDC_COMB_SpeciesIDs).ListIndexSet(488);
	return;
}
//Dijkstra计算时的进度反馈函数↓
bool DijkstraCalculatingCallBack(int Cycled_Times, int TotalCycleCurrent_Number, long userData)
{
	CBControl Static_Status = form1.Control(IDC_Static_Status);				//用CBcontrol定义状态显示文本框对象，方便后续代码编写操作
	/////////////////////////////////////////////////////////////
	//显示当前循环进度↓
	Static_Status.TextSet(Cycled_Times);											
	Static_Status.TextAdd(TEXT(" 次循环已完成"));

	if (TotalCycleCurrent_Number)														//若还有剩余循环未完成，则显示剩余进度
	{
		Static_Status.TextAdd(TEXT("，剩余"));
		Static_Status.TextAdd(TotalCycleCurrent_Number);
		Static_Status.TextAdd(TEXT(" 次循环……"));
	}
	else
		Static_Status.TextAdd(TEXT("。"));
	//显示当前循环进度↑
	/////////////////////////////////////////////////////////////
	DoEvents();																			//让出CPU检测用户是否选择中断

	return(!IfCancel_Sign);																//返回用户是否选择中断
}
//从数据库PathTasks表中获得任务并求得最短路径，然后保存到BetwResults表中的函数↓
void Button_KeepAnalysing()
{
	CBAdoRecordset rsTasks;																////用CBAdoRecordeset类来封装ADO中的RecordSet对象
	CBDijkstra DJMap;																	//包含DiJkstra算法的类
	int TasksTotal_Count = 0, TasksFinish_Count = 0;									//总任务数和已完成的任务数
	long ID_Begin = 0, ID_End = 0, Final_Distance = 0;									//起点、终点、最短路径
	CBControl Static_Count(IDC_Static_Count);											
	CBControl Static_Doing(IDC_Static_Doing);
	CBControl Static_Status(IDC_Static_Status);
	//用CBControl类来定义状态显示栏对象，用于替代“form1.Control(******)”，
	//方便后续代码编写及更改。
	IfCancel_Sign = false;																//标记用户未申请取消操作。
	rsTasks.Open(TEXT("SELECT COUNT(*) FROM PathTasks WHERE DoneState=0"));
	//使用SQL语句在数据库中查找，若访问数据库失败则返回值为0，若成功则返回值为1
	//从DoneState=0的位置开始计算最短路径，计算完成之后标记该行的DoneState=1
	TasksTotal_Count = (int)Val(rsTasks.GetField((long)0));				//读取总任务数。
	rsTasks.Close();																	//关闭记录集
	if (TasksTotal_Count <= 0)															//若总任务数小于等于0，则代表无任务或任务已全部完成
	{
		Static_Count.TextSet(TEXT(""));
		Static_Doing.TextSet(TEXT(""));
		Static_Status.TextSet(TEXT("PathTasks 表内无任务，或任务都已完成。"));
		//状态显示栏显示当前状态，提示用户无可继续的任务，并退出计算。
		return;
	}
	
	form1.Control(IDC_Button_Analysis).TextSet(TEXT("中断分析"));		//启动分析之后将分析按钮改为中断分析按钮
	form1.Control(IDC_Button_KeepAnalysing).EnabledSet(false);		//将继续分析按钮设为不可互动
	form1.Control(IDC_Button_PreViewResult).EnabledSet(false);		//将查看分析结果按钮设为不可互动
	//以上操作可以防止用户在启动分析之后误触其他按钮导致程序出错
	Static_Count.TextSet(TEXT("总任务数： "));									
	Static_Count.TextAdd(TasksTotal_Count);
	Static_Doing.TextSet(TEXT(""));
	Static_Status.TextSet(TEXT(""));
	Static_Status.TextSet(TEXT("正在构建网络……"));								
	//显示目前的分析任务状态
	rsTasks.Open(TEXT("SELECT COUNT(*) FROM ProtLinks"));							//使用SQL语句在数据库中查找，若访问数据库失败则返回值为0，若成功则返回值为1
	int LinksTotal_Count = (int)Val(rsTasks.GetField((long)0));			//取得总进度数
	rsTasks.Close();																	//关闭数据集
	form1.Control(IDC_PRO_Status).MaxSet(LinksTotal_Count);			
	form1.Control(IDC_PRO_Status).MinSet(0);
	form1.Control(IDC_PRO_Status).ValueSet(0);
	form1.Control(IDC_PRO_Status).VisibleSet(true);
	//通过进度条显示当前进度
	DJMap.Clear();																		//清空Dijkstra算法类中的邻接矩阵用来存储新的节点和边的数据。
	if (!rsTasks.Open(TEXT("SELECT * FROM ProtLinks")))							//使用SQL语句在数据库中查找，若访问数据库失败则返回值为0，若成功则返回值为1
	{
		Static_Status.TextSet(TEXT("构家网络失败。访问数据库表ProtLinks出错。"));//若打开表格失败，则报错并退出
		return;
	}
	int Num_Cnt = 0;																	//用来存储目前已经添加的关系数目
	while (!rsTasks.EOFRs())															//若rsTasks中的数据未读取完，则继续循环，若已读取完最后一行，则返回值为1，即终止循环
	{
		ID_Begin = (long)Val(rsTasks.GetField(TEXT("ENSPID1")));		//读取ENSPID1列的当前行为起点
		ID_End = (long)Val(rsTasks.GetField(TEXT("ENSPID2")));			//读取ENSPID2列的当前行为终点
		Final_Distance = (long)Val(rsTasks.GetField(TEXT("Distance")));
																						//先取当前两个蛋白间的路径为最短路，后续计算将更新改数据
		DJMap.AddNodesDist(ID_Begin, ID_End, Final_Distance);				//将上述取得的起点、终点、距离存入Dijkstra的地图中
		Num_Cnt++;																		//完成一条关系的载入并记录
		form1.Control(IDC_PRO_Status).ValueSet(Num_Cnt);				//将当前完成的任务数显示到进度条中
		if (Num_Cnt % 599 == 0)
		{
			//每599条关系存入就刷新一次状态显示文本框，并查看用户是否申请取消。
			Static_Doing.TextSet(ID_Begin);
			Static_Doing.TextAdd(TEXT(" -- "));
			Static_Doing.TextAdd(ID_End);
			Static_Doing.TextAdd(TEXT("  "));
			Static_Doing.TextAdd(Final_Distance);
			Static_Count.TextSet(Num_Cnt);
			Static_Count.TextAdd(TEXT(" / "));
			Static_Count.TextAdd(LinksTotal_Count);
			DoEvents();																	//让出CPU检查
			if (IfCancel_Sign)
			{
				//若用户申请取消操作，则显示用户中断并退出循环。
				Static_Status.TextSet(TEXT("用户中断。"));						
				break;
			}
		}
		rsTasks.MoveNext();																//该行数据处理完之后，移动到下一行。
	}
	rsTasks.Close();																	//关闭数据集
	if (!IfCancel_Sign)	
	{
		//若用户没有取消分析，则网络构建完成，并显示当前状态
		Static_Status.TextSet(TEXT("构建网络完成。"));
		Static_Count.TextSet(LinksTotal_Count);
		Static_Doing.SelTextSet(TEXT(""));
	}
	else
	{
		//若用户取消分析，则将各个按钮改回初始状态并退出分析
		form1.Control(IDC_Button_Analysis).TextSet(TEXT("分析"));		//将“取消分析”按钮改回“分析”按钮	
		form1.Control(IDC_Button_KeepAnalysing).EnabledSet(true);	//将“继续分析”按钮设置为可互动
		form1.Control(IDC_Button_PreViewResult).EnabledSet(true);	//将“查看分析结果”按钮设置为可互动
		form1.Control(IDC_PRO_Status).VisibleSet(false);			//将进度条设置为不可视，即隐藏进度条
		return;
	}
	////////////////////////////////////////////////////////////////
	//上方用于存储初始地图数据信息，下方用于完成分析各最短路径任务//
	////////////////////////////////////////////////////////////////
	long* FinalPathNodes;																//用于保存最终最短路径
	long PathNode_Number=0;																//最短路径中所包含的结点个数
	CBHashLK Hash_BetwKey;																//用哈希表保存[枢纽结点ID的键,介数的值]
	CBAdoRecordset rsBetw;																//用CBAdoRecordeset类作接口，来访问BetwResults表中的ADO RecordSet对象	

	form1.Control(IDC_PRO_Status).MaxSet(TasksTotal_Count);			
	form1.Control(IDC_PRO_Status).MinSet(0);
	form1.Control(IDC_PRO_Status).ValueSet(0);
	form1.Control(IDC_PRO_Status).VisibleSet(true);
	//进度条显示当前计算最短路径的进度。
	rsTasks.Open(TEXT("SELECT * FROM BetwResults"));								//使用SQL语句在数据库中查找，若访问数据库失败则返回值为0，若成功则返回值为1
	while (!rsTasks.EOFRs())															//一直读取到最后一行，若已读完最后一行则返回1
	{
		Hash_BetwKey.Add((long)Val(rsTasks.GetField(TEXT("Betw"))), (long)Val(rsTasks.GetField(TEXT("ENSPID"))), 0, 0, 0, 0, 0, false);
		//将Betw列当前行的内容，和ENSPID列当前行的内容存入哈希表中，方便后续操作。
		rsTasks.MoveNext();																//当前行保存完，移动到下一行
	}
	rsTasks.Close();																	//关闭数据集

	rsTasks.Open(TEXT("SELECT * FROM PathTasks WHERE DoneState=0 ORDER BY ENSPID1, ENSPID2") );
	//使用SQL语句在数据库中查找，若访问数据库失败则返回值为0，若成功则返回值为1
	//查询数据库获得各项任务，按ENSPID1的顺序排序，这样起点相同的求最短路经任务，
	//各结点的Wi只需计算一次，从而节省了计算时间。
	//排序时间为[O(nlogn)，O(n2)]而计算时间则节省了O(n2)
	while (!rsTasks.EOFRs())															//一直读取到最后一行，若已读完最后一行则返回1
	{
		ID_Begin = (long)Val(rsTasks.GetField(TEXT("ENSPID1")));		//取ENSPID1列当前行的数据作为起点
		ID_End = (long)Val(rsTasks.GetField(TEXT("ENSPID2")));			//取ENSPID2列当前行的数据作为终点
		if (!DJMap.NodeIndex(ID_Begin) || !DJMap.NodeIndex(ID_End))			//若起点不存在或终点不存在则跳过该行
		{
			rsTasks.MoveNext();
			continue;
		}
		//显示当前的计算状态及进度↓
		Static_Doing.TextSet(TEXT("正在计算最短路径："));							
		Static_Doing.TextAdd(ID_Begin);
		Static_Doing.TextAdd(TEXT(" -- "));
		Static_Doing.TextAdd(ID_End);

		Static_Count.TextSet(TasksFinish_Count);
		Static_Count.TextAdd(TEXT(" / "));
		Static_Count.TextAdd(TasksTotal_Count);
		//显示当前的计算状态及进度↑
		PathNode_Number = DJMap.GetDistance(ID_Begin, ID_End, Final_Distance, FinalPathNodes, DijkstraCalculatingCallBack);
		//计算取得两点间的最短路径
		Static_Status.TextSet(TEXT("最短路径计算已完成！"));						//提示当前结点计算已完成

		if (PathNode_Number > 0)														//若经过结点数大于0
		{
			Static_Status.TextSet(TEXT("路径中结点数="));							//显示路径中结点数
			Static_Status.TextAdd(PathNode_Number);
			for (int i = 1; i < PathNode_Number - 1; i++)								//除去i=0和i=pathnumber-1这两个起始节点
			{
				if (Hash_BetwKey.IsKeyExist(FinalPathNodes[i]))						//若哈希表中已存在此结点，则其值+1
					Hash_BetwKey.ItemSet(FinalPathNodes[i], Hash_BetwKey.Item(FinalPathNodes[i], false) + 1);
				else
					Hash_BetwKey.Add(1, FinalPathNodes[i]);						//若哈希表中未记录此结点，则添加此结点并设其值为1
			}
		}
		else                                                                            //若经过结点数小于等于0
			if (PathNode_Number < 0)													//若小于0，则显示这两点间无路径
				Static_Status.TextSet(TEXT("无路径。"));			
			else
			{
				Static_Status.TextSet(TEXT("计算出错："));						//若等于0，则表示计算出错，并将DJ类里的计算错误信息提供给用户
				Static_Status.TextAdd(DJMap.ErrDescription);
			}

		rsTasks.SetField(TEXT("DoneState"), 1);						//完成当前任务，将其DoneState标记为1
		TasksFinish_Count++;															//已完成的任务数+1
		form1.Control(IDC_PRO_Status).ValueSet(TasksFinish_Count);	//进度条显示当前进度
		DoEvents();																		//让出CPU，相应用户是否选择中断
		if (IfCancel_Sign)																//若用户选择中断，则终止循环
			break;

		rsTasks.MoveNext();																//该行数据处理完，移动到下一行
	}

	Static_Status.TextSet(TEXT("更新数据库BetwResults：删除旧数据……"));			//显示当前的操作
	ADOConn.Execute(TEXT("DELETE FROM BetwResults"));								//通过ADOConn对数据库进行SQL操作，该操作为删除BetwResults表
	Static_Status.TextSet(TEXT("更新数据库BetwResults：更新新数据……"));			//显示当前操作
	rsBetw.Open(TEXT("SELECT * FROM BetwResults"));								//使用SQL语句在数据库中查找，若访问数据库失败则返回值为0，若成功则返回值为1		
	for (int i = 1; i <= Hash_BetwKey.Count(); i++)
	{
		//更新BetwResults表中的数据
		rsBetw.AddNew();																//开始更新一组数据
		rsBetw.SetField(TEXT("ENSPID"), Hash_BetwKey.IndexToKey(i));
		//将ENSPID列当前行的值设置为：哈希表中的键。
		rsBetw.SetField(TEXT("Betw"), Hash_BetwKey.ItemFromIndex(i, false));
		//将Betw列当前行的值设置为：哈希表次键对应的值。
		rsBetw.Update();																//结束更新并保存
	}
	rsBetw.Close();																		//关闭数据集
	Static_Status.TextSet(TEXT("更新数据库 BetwResults 完成。"));					//显示当前进度

	rsTasks.Update();																	//更新rsTasks中的刚才刚添加进来并设置为“1”的数据，并加入数据库。
	rsTasks.Close();																	//关闭数据集
	
	if (!IfCancel_Sign)																	//若用户未选择取消中断
	{
		Static_Count.TextSet(TEXT("总任务数："));
		Static_Count.TextAdd(TasksTotal_Count);
		Static_Doing.TextSet(TEXT(""));
		Static_Status.TextSet(TEXT("分析任务已完成。"));
		//显示当前进度状态，提示用户分析任务已完成
	}
	else
		Static_Status.TextSet(TEXT("用户中断。"));								//若用户选择中断，则提示用户中断
	//此处，用户要么已经完成了全部分析任务，要么选择了中断
	//无论是哪种情况都需要恢复各个按钮状态
	form1.Control(IDC_Button_Analysis).TextSet(TEXT("分析"));			
	form1.Control(IDC_Button_KeepAnalysing).EnabledSet(true);
	form1.Control(IDC_Button_PreViewResult).EnabledSet(true);
	form1.Control(IDC_PRO_Status).VisibleSet(false);
	return;
}
//“分析”按钮的功能函数，分别编写该按钮两个状态下的功能↓
void Button_Analysis()
{	
	//===================================//
	//若此时按钮为“中断分析”状态↓
	if (_tcscmp(form1.Control(IDC_Button_Analysis).Text(), TEXT("中断分析")) == 0)
	{
		//则询问用户是否要继续中断操作，若用户选择是，则将中断标记设置为true，以命令程序进行中断操作，并且推出当前功能。
		if (MsgBox(TEXT("分析尚未完成,确定要取消吗？\r\n已完成的分析结果会在数据库中保存。\n中断后您可以下次继续进行分析。"), TEXT("中断分析"), mb_YesNoCancel, mb_IconQuestion) == idYes)
			IfCancel_Sign = true;
		return;
	}
	//若此时按钮为“中断分析”状态↑
	//===================================//
	//若此时按钮为“分析”状态↓
	//询问用户是否要从头分析数据，并清空已经处理过的数据信息。
	if (MsgBox(TEXT("此操作将重新读取 ENSP数据集文件，容纳后重建数据库的待分析待办列表，先前的分析将被请出，是否继续？"), TEXT("确认重新分析"), mb_OkCancel, mb_IconQuestion) != idOk)
		return;
	
	LPTSTR Size_File = form1.Control(IDC_COMB_EnspLisFiles).Text();			//储存列表IDC_COMB_EnspLisFiles的数据信息
	CBControl Static_Count(IDC_Static_Count);
	CBControl Static_Doing(IDC_Static_Doing);
	CBControl Static_Status(IDC_Static_Status);
	//用CBControl类来定义状态显示栏对象，用于替代“form1.Control(******)”，
	//方便后续代码编写及更改。
	if (*Size_File == 0)																//若列表IDC_COMB_EnspLisFiles为空，则报错并退出
	{
		MsgBox(TEXT("请选择ENSP数据集文件！"), TEXT("未选择文件"), mb_OK, mb_IconExclamation);
		return;
	}
	Static_Status.TextSet(TEXT("清空 TarProts 表……"));							//显示当前操作
	ADOConn.Execute(TEXT("DELETE FROM TarProts"));									//清空TarProts表

	Static_Status.TextSet(TEXT("读取 ENSP数据集文件……"));						//显示即将进行的操作
	
	CBReadLinesEx Aiming_File;															//用CBReadLinesEx类来存储目标文件
	CBAdoRecordset rs;																	
	LPTSTR Size_Line;																	
	tstring String_Field;																//存储目标字符串
	LPTSTR String_SQL = NULL;															//存储SQL语句
	if (!rs.Open(TEXT("SELECT * FROM TarProts")))									//使用SQL语句在数据库中查找，若访问数据库失败则返回值为0，若成功则返回值为1
		return;
	if (!Aiming_File.OpenFile(Size_File))										//打开目标文件，若目标文件打开失败，则退出
		return;
	while(!Aiming_File.IsEndRead())														//若目标文件未读取到最后一行则持续循环
	{
		Aiming_File.GetNextLine(Size_Line);										//读取下一行字符串
		if (Aiming_File.IsErrOccured())													//若出错，则退出该操作
			return;

		String_Field = Size_Line;														//将Size_Line赋值给Field，用Field判断前4个字符是否合法
		if (String_Field.substr(0, 4) == TEXT("ENSP"))						//若前四个字符是“ENSP”，则执行下面操作
		{
			rs.AddNew();																//为TarProts表更新数据
			rs.SetField((long)0, Size_Line + 4);							//将前4个字符去掉，即删掉“ENSP”后存入到TarProts表中
			rs.Update();																//结束并保存更新操作
		}
	}
	rs.Close();																			//关闭数据集
	Static_Status.TextSet(TEXT("数据库 TarProts 表保存完成。"));					//显示当前状态

	ADOConn.Execute(TEXT("DROP TABLE PathTasks"));									//若已有此表格，则删除已有的表格
	String_SQL= TEXT("SELECT T1.ENSPID AS ENSPID1, T2.ENSPID AS ENSPID2, 0 AS DoneState INTO PathTasks FROM TarProts T1, TarProts T2 WHERE T1.ENSPID < T2.ENSPID GROUP BY T1.ENSPID, T2.ENSPID");
	//生成最短路径任务表PathTasks
	//共三列，分别为起点、终点、最短距离
	ADOConn.Execute(String_SQL);
	//执行SQL语句
	bool If_PathTasksRun=true;															//标记PathTasks表生成成功
	if (!rs.Open(TEXT("SELECT * FROM PathTasks")))									//使用SQL语句在数据库中查找，若访问数据库失败则返回值为0，若成功则返回值为1
		If_PathTasksRun = false;														//若询问失败，则代表PathTasks表生成失败
	else
		if (rs.EOFRs())																	//若表格为空，也表示生成失败
			If_PathTasksRun = false;
	if (!If_PathTasksRun)																//若生成失败，则报错并退出
	{
		MsgBox(TEXT("PathTasks 表生成可能失败！\n请在此处设置断点，然后将变量String_SQL的内容从监视窗口中复制粘贴到Access的查询中，在Acess中运行此查询检查……"));
		return;
	}

	ADOConn.Execute(TEXT("DELETE FROM BetwResults"));								//清空BetwResults表
	Button_KeepAnalysing();																//任务已准备好，执行分析操作
	return;
}
//浏览本地文件夹并添加ENSP文件按钮↓
void Button_BrowsENSP()
{
	//此处内容与Button_BrowsPPI函数部分内容基本相同，故不再过多注释
	LPTSTR Size_File;
	OsdSetFilter(TEXT("文本文件(*.txt)|*.txt|数据文件(*.dat)|*.dat"), true);
	Size_File = OsdOpenDlg(form1.hWnd(), TEXT("请选择ENSP蛋白列表文件"));
	if (*Size_File)
	{
		form1.Control(IDC_COMB_EnspLisFiles).AddItem(Size_File);
		form1.Control(IDC_COMB_EnspLisFiles).TextSet(Size_File);
	}
	return;
}
//通过鼠标拖动为EnspLisFiles列表框添加文件函数↓
void COMBEnspLisFiles_FilesDrop(int ptrArrayFiles, int ENSP_Cnt, int x, int y)
{
	TCHAR** Files = (TCHAR**)ptrArrayFiles;
	//将ptrArrFiles转换为TCHAR ** 类型，即二维数组，这样可以使files里的内容保存到二维数组中
	//也就是字符串的一维数组，通过∑file[i]，依次取得file文件中的各个字符串
	//这些字符串就是用户所拖动的文件中，被拖动控件上的各个文件名
	form1.Control(IDC_COMB_EnspLisFiles).AddItem(Files[1]);
	form1.Control(IDC_COMB_EnspLisFiles).TextSet(Files[1]);
	//此处只打开一个文件file[1]，将其添加并显示到组合框
	return;
}
//释放内存并退出主程序的函数↓
void Button_ShutOff()
{
	form1.UnLoad();																		//释放程序内存
	return;
}

int main()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//将各控件的功能函数与其控件相关功能相连接↓
	form1.EventAdd(IDC_Button_Brows, eCommandButton_Click, Button_BrowsPPI);
	form1.EventAdd(IDC_Button_ExtractSpeciesID, eCommandButton_Click, Button_ExtractSpeciesID);
	form1.EventAdd(IDC_COMB_PPIFiles, eFilesDrop, COMBPPIFiles_FilesDrop);
	form1.EventAdd(IDC_COMB_EnspLisFiles, eFilesDrop, COMBEnspLisFiles_FilesDrop);
	form1.EventAdd(IDC_Button_Analysis, eCommandButton_Click, Button_Analysis);
	form1.EventAdd(IDC_Button_KeepAnalysing, eCommandButton_Click, Button_KeepAnalysing);
	form1.EventAdd(IDC_Button_BrowsENSP, eCommandButton_Click, Button_BrowsENSP);
	form1.EventAdd(IDC_Button_PreViewResult, eCommandButton_Click, Button_PreViewResult);
	form1.EventAdd(IDC_Button_ShutOff, eCommandButton_Click, Button_ShutOff);

	EventsMapFrmView();																	//加载结果预览界面的各控件及其功能
	//将各控件的功能函数与其控件相关功能相连接↑
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	form1.IconSet(IDI_ICON1);


	if (!ADOConn.Open(TEXT("PPI.accdb")))
		MsgBox(ADOConn.ErrorLastStr(), TEXT("Sorry!main()里的AD0Conn.Open()数据库连接失败！"), mb_OK, mb_IconExclamation);
	
																	//欢迎界面，并且预处理程序	
	//Preset_Welcome();

	Button_PreViewResult();
	while (1)
	{
		if (BREAK_SIG)
			break;
		DoEvents();
	}

	form1.Show();
	ShowSpeciesID();																	//从目标数据库获得所有的物种ID并显示到IDC_COMB_SpeciesIDs列表框

	return 0;
}