#include "resource.h"
#include "BForm.h"
#include "BHashStrK.h"
#include "mdlOpenSaveDlg.h"
#include "BADO.h"


CBForm Form_FrmView(IDD_Form_FrmView);

bool BREAK_SIG = false;
//�����ý���ġ�main��������
void EventsMapFrmView();																		
//��ʾ�Ѽ��㴦��õĽ�����ô��ڽ����
void FrmView_Load()																				
{
	CBControl List_Result = Form_FrmView.Control(IDC_LIST_Result);					//��CBControl��������״̬��ʾ���������������form1.Control(******)����������������д�����ġ�					
	List_Result.ListViewAddColumn(TEXT("Protein ENSP"), 120);
	List_Result.ListViewAddColumn(TEXT("Gene Name"), 80);
	List_Result.ListViewAddColumn(TEXT("Betweenness"), 80);
	List_Result.ListViewGridLinesSet(true);
	List_Result.ListViewFullRowSelectSet(true);
	//���б��IDC_LIST_Result���и��ģ��Ը������û��Ķ������
	//����Ϊ���У���һ��ΪProtein ENSP���ڶ���ΪGene Name��������ΪBetweenness
	CBAdoRecordset rs;
	if (!rs.Open(TEXT("SELECT 'ENSP' & Right(String(11,'0') & BetwResults.ENSPID,\
							11) AS ProtENSP, ENSPGenes.GeneName, BetwResults.Betw\
							FROM ENSPGenes RIGHT JOIN BetwResults ON \
							ENSPGenes.ENSPID = BetwResults.ENSPID \
							ORDER BY BetwResults.Betw DESC; ")))
		return;
	//ʹ��SQL��������ݿ��в��ң����������ݿ�ʧ���򷵻�ֵΪ0�����ɹ��򷵻�ֵΪ1
	//�˴���ѯ�����е�SQL���String(11,'0')�����ǻ�ȡ����11��0��ɵ��ַ����� & Ϊ���������ַ���
	//RIGHT��ȡ���Ӻ���ַ����ĺ�11���ַ������Ӷ�ʵ����BetwResults.ENSPIDǰ�油0������11λ��
	while (!rs.EOFRs())																			//��δ��ȡ�����һ�У������ѭ�������Ѷ�ȡ�����һ�У��򷵻�1
	{
		//ȡBetwResults���еĽ����������ʾ���б�IDC_LIST_Result��
		int RollsNumber = List_Result.AddItem(rs.GetField(TEXT("ProtENSP")));	//ȡProtENSP�еĵ�ǰ��������ӵ��б�RollsNumber�еĵ�1�С�
		List_Result.ItemTextSet(RollsNumber, rs.GetField(TEXT("GeneName")), 2);
		//ȡGeneName�еĵ�ǰ��������ӵ��б�RollsNumber�еĵ�2�С�
		List_Result.ItemTextSet(RollsNumber, rs.GetField(TEXT("Betw")), 3);
		//ȡBetw�еĵ�ǰ��������ӵ��б�RollsNumber�еĵ�3�С�
		rs.MoveNext();																			//��ǰ�д�����ϣ��ƶ�����һ��
	}
	return;
}
//�����С���ı�ʱ���ú�������ʹ�����ؼ����Ŵ��ڴ�С�ı仯���仯��
void FrmView_Resize()
{
	Form_FrmView.Control(IDC_LIST_Result).Move(0, 0, Form_FrmView.ClientWidth() - Form_FrmView.Control(IDC_Button_Save).Width() - 20, Form_FrmView.ClientHeight());
	Form_FrmView.Control(IDC_Button_Save).Move(Form_FrmView.ClientWidth() - Form_FrmView.Control(IDC_Button_Save).Width() - 10, Form_FrmView.ClientHeight() - Form_FrmView.Control(IDC_Button_Close).Height() * 2 - 20);
	Form_FrmView.Control(IDC_Button_Close).Move(Form_FrmView.ClientWidth() - Form_FrmView.Control(IDC_Button_Close).Width() - 10, Form_FrmView.ClientHeight() - Form_FrmView.Control(IDC_Button_Close).Height() - 10);
	return;
}
//���水ť�Ĺ��ܡ�
void Button_Save()
{
	CBControl List_Result = Form_FrmView.Control(IDC_LIST_Result);				//��CBControl��������״̬��ʾ���������������form1.Control(******)����������������д�����ġ�
	LPTSTR Size_File;
	OsdSetFilter(TEXT("�ı��ļ�(*.txt)"), true);							//�����ı����ļ��ĸ�ʽ������׺����			
	Size_File = OsdSaveDlg(Form_FrmView.hWnd(), TEXT("Analysis_Result.txt"), TEXT("������Ҫ����������ļ�"));
	//ѡ�񱾵��ļ���ѡ���Ĵ�������
	if (*Size_File == 0)																	//�������ı�Ϊ�����޷����棬ֱ���˳�
		return;

	pApp->MousePointerGlobalSet(IDC_Wait);												//���ָ���ڱ���ʱ��Ϊ�ȴ�ͼ�꣬����ʾ�û���������ִ�б������
	HANDLE Open_Files = EFOpen(Size_File);
	int Item_Count = List_Result.ListCount();
	if (Open_Files == INVALID_HANDLE_VALUE)
		return;

	for (int i = 1; i <= Item_Count; i++)	
	{
		//����i�д�ӡ��Ŀ���ļ����еĵ�i��
		//ͬ���е��ֶ��÷ָ���������ÿ��������л��з�����
		EFPrint(Open_Files, List_Result.ItemText(i, 1), EF_LineSeed_None);
		EFPrint(Open_Files, TEXT("\t"), EF_LineSeed_None);
		EFPrint(Open_Files, List_Result.ItemText(i, 2), EF_LineSeed_None);
		EFPrint(Open_Files, TEXT("\t"), EF_LineSeed_None);
		EFPrint(Open_Files, List_Result.ItemText(i, 3), EF_LineSeed_CrLf);
	}
	EFClose(Open_Files);																//�ر�Ŀ���ļ���
	pApp->MousePointerGlobalSet(0);												//���ָ���ڱ���ʱ��Ϊ����ͼ�꣬����ʾ�û���������ɹ����
	return;
}
//�رս��Ԥ�����ڰ�ť��
void Button_Close()
{
	if (MsgBox(TEXT("ȷ��Ҫ������"), TEXT("��ã�"), mb_YesNoCancel, mb_IconNone) == idYes)
	{
		Form_FrmView.UnLoad();
		BREAK_SIG = true;
		return;
	}
	Form_FrmView.UnLoad();															//�ͷŵ�ǰ���ڵ��ڴ沢�رմ���											
	return;
}
//�ô��ڽ���ġ�main���������������Ӹ����ؼ��Ĺ����뺯����
void EventsMapFrmView()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//�����ؼ��Ĺ��ܺ�������ؼ���ع��������ӡ�
	Form_FrmView.EventAdd(0, eForm_Load, FrmView_Load);
	Form_FrmView.EventAdd(0, eForm_Resize, FrmView_Resize);
	Form_FrmView.EventAdd(IDC_Button_Save, eCommandButton_Click, Button_Save);
	Form_FrmView.EventAdd(IDC_Button_Close, eCommandButton_Click, Button_Close);
	//�����ؼ��Ĺ��ܺ�������ؼ���ع��������ӡ�
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	return;
}