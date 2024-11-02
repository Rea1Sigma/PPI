#include "resource.h"
#include "BForm.h"
#include "BHashStrK.h"
#include "mdlOpenSaveDlg.h"
#include "BADO.h"


CBForm Form_FrmView(IDD_Form_FrmView);

bool BREAK_SIG = false;
//声明该界面的“main”函数↓
void EventsMapFrmView();																		
//显示已计算处理好的结果到该窗口界面↓
void FrmView_Load()																				
{
	CBControl List_Result = Form_FrmView.Control(IDC_LIST_Result);					//用CBControl类来定义状态显示栏对象，用于替代“form1.Control(******)”，方便后续代码编写及更改。					
	List_Result.ListViewAddColumn(TEXT("Protein ENSP"), 120);
	List_Result.ListViewAddColumn(TEXT("Gene Name"), 80);
	List_Result.ListViewAddColumn(TEXT("Betweenness"), 80);
	List_Result.ListViewGridLinesSet(true);
	List_Result.ListViewFullRowSelectSet(true);
	//对列表框IDC_LIST_Result进行更改，以更方便用户阅读结果。
	//共分为三列，第一列为Protein ENSP，第二列为Gene Name，第三列为Betweenness
	CBAdoRecordset rs;
	if (!rs.Open(TEXT("SELECT 'ENSP' & Right(String(11,'0') & BetwResults.ENSPID,\
							11) AS ProtENSP, ENSPGenes.GeneName, BetwResults.Betw\
							FROM ENSPGenes RIGHT JOIN BetwResults ON \
							ENSPGenes.ENSPID = BetwResults.ENSPID \
							ORDER BY BetwResults.Betw DESC; ")))
		return;
	//使用SQL语句在数据库中查找，若访问数据库失败则返回值为0，若成功则返回值为1
	//此处查询命令中的SQL语句String(11,'0')功能是获取连续11个0组成的字符串， & 为连接两个字符串
	//RIGHT获取连接后的字符串的后11个字符串，从而实现在BetwResults.ENSPID前面补0，凑足11位。
	while (!rs.EOFRs())																			//若未读取到最后一行，则持续循环，若已读取到最后一行，则返回1
	{
		//取BetwResults表中的结果，依次显示到列表IDC_LIST_Result中
		int RollsNumber = List_Result.AddItem(rs.GetField(TEXT("ProtENSP")));	//取ProtENSP列的当前行数据添加到列表RollsNumber行的第1列。
		List_Result.ItemTextSet(RollsNumber, rs.GetField(TEXT("GeneName")), 2);
		//取GeneName列的当前行数据添加到列表RollsNumber行的第2列。
		List_Result.ItemTextSet(RollsNumber, rs.GetField(TEXT("Betw")), 3);
		//取Betw列的当前行数据添加到列表RollsNumber行的第3列。
		rs.MoveNext();																			//当前行处理完毕，移动到下一行
	}
	return;
}
//窗体大小被改变时，该函数可以使各个控件随着窗口大小的变化而变化↓
void FrmView_Resize()
{
	Form_FrmView.Control(IDC_LIST_Result).Move(0, 0, Form_FrmView.ClientWidth() - Form_FrmView.Control(IDC_Button_Save).Width() - 20, Form_FrmView.ClientHeight());
	Form_FrmView.Control(IDC_Button_Save).Move(Form_FrmView.ClientWidth() - Form_FrmView.Control(IDC_Button_Save).Width() - 10, Form_FrmView.ClientHeight() - Form_FrmView.Control(IDC_Button_Close).Height() * 2 - 20);
	Form_FrmView.Control(IDC_Button_Close).Move(Form_FrmView.ClientWidth() - Form_FrmView.Control(IDC_Button_Close).Width() - 10, Form_FrmView.ClientHeight() - Form_FrmView.Control(IDC_Button_Close).Height() - 10);
	return;
}
//保存按钮的功能↓
void Button_Save()
{
	CBControl List_Result = Form_FrmView.Control(IDC_LIST_Result);				//用CBControl类来定义状态显示栏对象，用于替代“form1.Control(******)”，方便后续代码编写及更改。
	LPTSTR Size_File;
	OsdSetFilter(TEXT("文本文件(*.txt)"), true);							//保存后的本地文件的格式（即后缀名）			
	Size_File = OsdSaveDlg(Form_FrmView.hWnd(), TEXT("Analysis_Result.txt"), TEXT("请输入要保存的数据文件"));
	//选择本地文件的选择框的窗口名。
	if (*Size_File == 0)																	//若保存文本为空则无法保存，直接退出
		return;

	pApp->MousePointerGlobalSet(IDC_Wait);												//鼠标指针在保存时变为等待图标，已提示用户程序正在执行保存操作
	HANDLE Open_Files = EFOpen(Size_File);
	int Item_Count = List_Result.ListCount();
	if (Open_Files == INVALID_HANDLE_VALUE)
		return;

	for (int i = 1; i <= Item_Count; i++)	
	{
		//将第i行打印到目标文件夹中的第i行
		//同意行的字段用分隔符隔开，每行输出完有换行符隔开
		EFPrint(Open_Files, List_Result.ItemText(i, 1), EF_LineSeed_None);
		EFPrint(Open_Files, TEXT("\t"), EF_LineSeed_None);
		EFPrint(Open_Files, List_Result.ItemText(i, 2), EF_LineSeed_None);
		EFPrint(Open_Files, TEXT("\t"), EF_LineSeed_None);
		EFPrint(Open_Files, List_Result.ItemText(i, 3), EF_LineSeed_CrLf);
	}
	EFClose(Open_Files);																//关闭目标文件夹
	pApp->MousePointerGlobalSet(0);												//鼠标指针在保存时变为正常图标，以提示用户保存操作成功完成
	return;
}
//关闭结果预览窗口按钮↓
void Button_Close()
{
	if (MsgBox(TEXT("确定要继续吗？"), TEXT("你好！"), mb_YesNoCancel, mb_IconNone) == idYes)
	{
		Form_FrmView.UnLoad();
		BREAK_SIG = true;
		return;
	}
	Form_FrmView.UnLoad();															//释放当前窗口的内存并关闭窗口											
	return;
}
//该窗口界面的“main”函数，用于连接各个控件的功能与函数。
void EventsMapFrmView()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//将各控件的功能函数与其控件相关功能相连接↓
	Form_FrmView.EventAdd(0, eForm_Load, FrmView_Load);
	Form_FrmView.EventAdd(0, eForm_Resize, FrmView_Resize);
	Form_FrmView.EventAdd(IDC_Button_Save, eCommandButton_Click, Button_Save);
	Form_FrmView.EventAdd(IDC_Button_Close, eCommandButton_Click, Button_Close);
	//将各控件的功能函数与其控件相关功能相连接↑
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	return;
}