#include "resource.h"
#include "BForm.h"
#include "BHashStrK.h"
#include "mdlOpenSaveDlg.h"
#include "BADO.h"
#include "BDijkstra.h"
#include "BReadLinesEx.h"

CBForm form1(ID_form1);
//��������һCPP�ļ��й�����ʾ���Ԥ������ĺ����������ڴ�cpp�ļ���ʹ�øú���
//����Ҫʹ��extern +�������������������������󡢺�������˲�����
extern CBForm Form_FrmView;																
extern void EventsMapFrmView();															//���ؽ��Ԥ������ĸ��ؼ����书��									
extern bool BREAK_SIG;

bool IfCancel_Sign = false;																//���ڱ���û��Ƿ�����ȡ����ǰ������

//����Ԥ�����
void Preset_Welcome()
{
	form1.Control(IDC_Static_Doing).TextSet(TEXT("������������밴���Ⱥ�˳����������\n1.����PPI�����ļ�����ȡ��Ӧ������ID�����ݿ⡣\n2.����ENSP���ݼ��ļ�\n3.���������ť��"));
																						//Static_Doing��ʾ��ǰ״̬���������û����в�����
	form1.Control(IDC_Static_Status).TextSet(TEXT(""));					//���Static_Status�ı�
	form1.Control(IDC_Static_Count).TextSet(TEXT(""));					//���Static_Count�ı�
	//MsgBox(TEXT("�ó���ּ�ڣ����Է������׼�����ù�ϵ����������������ϵ��\nQQ:1537361071\n����:1537361071@qq.com"), TEXT("��ӭ��"), mb_OK, mb_IconNone);
		
	if (MsgBox(TEXT("�ó���ּ�ڣ����Է������׼�����ù�ϵ����������������ϵ��\nQQ:1537361071\n����:1537361071@qq.com"), TEXT("��ӭ��"), mb_YesNoCancel, mb_IconNone) != idYes)
	{
		return;
	}
	//����ӭ���û��Ի���
	return;
}
//�������Ԥ����ť��
void Button_PreViewResult()
{
	Form_FrmView.Show();																//��ʾ�������Ԥ�����ڡ�
}
/*PPI��ȡ��������Start_Positionλ�ÿ�ʼ��ȡ�������ݣ�һֱ�������ݿ�ͷ��ΪString_Find���У�ͬʱ�Ե�ǰ��������
����ȡд��ɹ����򷵻ز�����ɵ�������������ȡд��ʧ�ܻ������ݿɶ�ȡд�룬�򷵻�0��*/
long ReadPPIToDB(LPCTSTR Size_File, LONGLONG Start_Position, LPCTSTR String_Find)
{
	CBAdoRecordset rs;																	//�����µ�CBAdoRecordeset�ṹ��������������ACESS���ݿ�Ĳ�����
	if (!rs.Open(TEXT("SELECT * FROM ProtLinks")))									//��δ�ɹ���Ŀ��ACESS���ݿ����д�룬����ֹ���򲢱���
	{
		MsgBox(TEXT("��Acess���ݿ��е�ProtLinksд��ʧ�ܣ�"), TEXT("Acessд��ʧ��"), mb_OK, mb_IconExclamation);
		return 0;
	}
	if (!rs.EOFRs())																	//EOF()���ش�ʱ�Ƿ������һ�У���������EOF=1������EOF=0��
	{
		if (MsgBox(TEXT("���ݿ��е� ProtLinks �������м�¼��Ҫ����������¼�¼������ɾ��ԭ�м�¼��\r\n�Ƿ�ɾ��ԭ�м�¼��"), TEXT("������ݿ��"), mb_YesNoCancel, mb_IconExclamation) != idYes)										
			return 0;																	//ѯ���Ƿ�Ҫ��ͷ��ʼ��¼����������
		pApp->MousePointerGlobalSet(IDC_Wait);										//ָ���Ϊ�ȴ����������û���ʱ�������ڽ���ɾ������
		form1.Control(IDC_Static_Status).TextSet(TEXT("����ɾ�� ProtLinks �������м�¼����"));
																						//��ʾ�����������ProtLinks������
		ADOConn.Execute(TEXT("DELETE FROM ProtLinks"));							//��Acess������SQL��������ProtLinks���
		form1.Control(IDC_Static_Status).TextSet(TEXT("�����ԭ���ݼ�¼"));
																						//��ʾ���˳�����
		pApp->MousePointerGlobalSet(0);										//ָ���س��棬��ʾ��ղ���˳����ɡ�	
	}

	char * Aiming_Find = StrConvFromUnicode(String_Find);						//���µ�ָ��ָ��Ҫ���ҵı���
	long Length_Find = strlen(Aiming_Find);											//Ŀ��ΪANSI��ʽ��������strlen()������ȡҪ���ҵ����ݳ���	
	CBReadLinesEx Aiming_File;															//
	tstring String_Field;																//�����ж�ĳ���ֶε�����
	long Imported_Count = 0;															//��¼�ѵ������������
	LPTSTR Size_Line;																	//
	TCHAR ** s;
	int n;
	CBControl Static_Status(IDC_Static_Status);											//��CBControl������Ҫ����Static_Status�飬�����������
	if (!Aiming_File.OpenFile(Size_File))										//��Ŀ���ļ�����δ�ɹ�����ֱ���˳�
		return 0;	
	Aiming_File.SeekFile(Start_Position);											//���Ѽ�¼����ʼ�㿪ʼ��ȡ�ļ�
	while (!Aiming_File.IsEndRead())													//����ȡ�����һ����ֹͣѭ��
	{
		Aiming_File.GetNextLine(Size_Line);										//�ƶ�����һ������
		if (Aiming_File.IsErrOccured())													//�����������޷���ȡ�����˳�ѭ��
			return 0;
		n = Split(Size_Line, s, TEXT(" "));					//�������������ݰ��տո�ָ���������s�������У���s[1]��ʼ���δ��棬�������зָ���������������ص�����n��
		if (n < 3)																		//ÿ��Ӧ���������ݣ�����������С��3�������Ч����������ֱ�ӽ�����һ��ѭ��
			continue;
		String_Field = s[1];															//���������涨���String_Field�������ж�����ȡ�����Ƿ�Ϸ�
		if (String_Field.substr(0, Length_Find) != String_Find)				//����ȡ�Ĳ��ǡ�9606.ENSP�������ļ�����ֹѭ��
			break;
		rs.AddNew();																	//��ʼ��������
		rs.SetField(TEXT("ENSPID1"), (int)Val(s[1] + Length_Find));
																						//��λ��9606.ENSP֮������֣���s[1]�ⲿ�����������εķ�ʽ��ӵ�ENSPID1�еĵ�ǰ��
		rs.SetField(TEXT("ENSPID2"), (int)Val(s[2] + Length_Find));
																						//��λ��9606.ENSP֮������֣���s[1]�ⲿ�����������εķ�ʽ��ӵ�ENSPID1�еĵ�ǰ��
		rs.SetField(TEXT("Distance"), 1000 - (int)Val(s[3])); 
		//ǰ���Ѿ�������õĵ������ù�ϵ��ֵ�����浽��s[3]���У�������Խǿ����s[3]Խ����s[3]��[0,1000],
		//����Ŀ����������Ĭ��Dijkstraģ�����������������·�����޷����·����������1000-��������֮���s[3],
		//�������������׽���ľ��룬��ʱ����Խ�̴������������׹�����Խǿ��������ͨ��Dijkstra�㷨���������������
		//��������·������Ϊ���������׽��֮�����̾��룬���ֵԽС���������Խǿ��
		
		rs.Update();																	//ȷ������SetField�Ķ�
		Imported_Count++;																//����ɵ�������+1
		if (Imported_Count % 300 == 0)													//ÿ300�����ݸ��£�ˢ��һ�ν�����ʾ�ı�
		{
			Static_Status.TextSet(TEXT("�ѵ���: "));
			Static_Status.TextAdd(Imported_Count);
			Static_Status.TextAdd(TEXT(" ����¼\r\n"));
			Static_Status.TextAdd(Size_Line);
			DoEvents();																	//�ó�CPU
			if (IfCancel_Sign)															//����û��Ƿ����жϣ����û�ѡ��ֹͣ����ֹͣ����
			{
				Static_Status.TextSet(TEXT("�û��жϡ�"));
				break;
			}
		}
	}
	rs.Close();																			//�رռ�¼�������������浱ǰ���ֲ���
	if (!IfCancel_Sign)																	//���û�δѡ���жϣ�����ʾ����ɵĹ���
	{
		Static_Status.TextSet(TEXT("���ݵ�����ɣ������� "));
		Static_Status.TextAdd(Imported_Count);
		Static_Status.TextAdd(TEXT(" ����¼"));
	}
	return Imported_Count;																//�����Ѹ��µ���������
}
//��λĿ�����ݣ�9606.ENSP��λ�õĺ�����
LONGLONG FindPos_PPIFile(HANDLE Open_Files, LPCTSTR Size_File)
{
	//��Open_Files�ļ��в��ң����ַ���Siz_File��ͷ������
	char * Aiming_Find = StrConvFromUnicode(Size_File);							//��Ŀ������ת�浽Aiming_Find�ַ�����
	long Length_Finding = strlen(Aiming_Find);										//Ŀ��ΪANSI��ʽ��������strlen()������ȡҪ���ҵ����ݳ���	
	char Buffer[131072];																//������
	bool If_Found = false;																//����Ƿ��ҵ�Ŀ��
	long ReadBytes = 0, ReadCycled = 0, i, k;											
	LONGLONG File_TotalBytes = EFLOF(Open_Files);									//�ļ������ֽ���
	LONGLONG File_Position = 0;															//��ǰ��д����λ��		
	
	while (File_Position < File_TotalBytes)
	{
		ReadBytes = EFGetBytes(Open_Files, File_Position, Buffer, sizeof(Buffer));
			//��File_Positionλ�ÿ�ʼ�����Ͻ�Open_Files����ַ�����Buffer���飬ֱ���������ȡ���ļ����һ���ֽ�ֹͣ��
			//��δ�ɹ���ȡ�򷵻�ֵΪ�������ɹ���ȡ�򷵻�ֵΪ�˴δ���Buffer������ַ���Ŀ��
		if (ReadBytes <= 0)																//��EFGetBytes������ȡ�ļ�ʧ�ܣ��򷵻�ֵС��0����ֹѭ��
			break;
		for (i = 0; i < ReadBytes; i++)
		{
			if (Buffer[i] == 10 || (File_Position == 0 && i == 0))						//�ҵ�\n���ļ���ͷ�ĵ�0���ڴ�λ�ã�����һ���ֽڣ�
			{
				if (Buffer[i] == 10)
					i++;																//��buff[i]��\n�������һ���ڴ�λ�ÿ�ʼ����9606.ENSP�ַ���
				if (ReadBytes - i < Length_Finding)										//ReadBytes-i��ʣ���ֽ���Ŀ����ʣ���ֽ���Ŀ�Ѿ����㡮9606.ENSP���ַ�����ռ���ֽ���Ŀ
				{
					File_Position -= (ReadBytes - i);									//��Ŀ��ָ����������Ĳ��ҹ��̣���ʡ�˲���ʱ��
					break;																//�˳�ѭ����������һ���ַ���
				}
				for (k = 0; k < Length_Finding; k++)									//Ѱ������9606.ENSP���ַ�������
				{
					if (Buffer[i + k] != Aiming_Find[k])								//��������һ���������������˳�ѭ�����У���ʡ�˼���ʱ��
						break;
				}
				if (k >= Length_Finding)												//���k>=Ŀ���ַ������ȣ�����Ŀ���ַ���λ�õ�ÿһ���ֽڶ�����Ҫ��ֱ����һ�вŲ�����Ҫ�󣬼��ѳɹ��ҵ�Ŀ��λ��
				{
					If_Found = true;													//������ҵ������˳�ѭ��
					break;
				}
			}
		}
		if (If_Found)																	//If_Found==1								
			break;																		//���ѳɹ��ҵ�Ŀ��λ�þ���ֹ���ҹ���
		File_Position += ReadBytes;														//Ŀ��λ�þ͵��ڳ�ʼλ�ã��Ѳ�ѯ���ֽ���
		ReadCycled++;																	//ѭ����+1
		if (ReadCycled % 50 == 0)														//ÿ50��ѭ�����һ�ε�ǰ��ѯ����
		{
			form1.Control(IDC_Static_Status).TextSet(TEXT("����"));		
			form1.Control(IDC_Static_Status).TextAdd((double)(File_Position / 1024 / 1024));
			form1.Control(IDC_Static_Status).TextAdd(TEXT(" MB / "));
			form1.Control(IDC_Static_Status).TextAdd((double)(File_TotalBytes / 1024 / 1024));
			form1.Control(IDC_Static_Status).TextAdd(TEXT(" MB ���� "));
			//���ϲ���Ϊ���ھ�̬�ı��������ǰ�Ĳ�ѯ���ȡ�
			DoEvents();																	//�ó�CPU
			if (IfCancel_Sign)															//���û���ֹ
			{
				form1.Control(IDC_Static_Status).TextSet(TEXT("�û��ж϶�ȡ��"));
				break;																	//��ʾ�û��ж϶�ȡ���˳�ѭ��
			}
		}
	}
	if (If_Found)																		//�����ҵ�Ŀ��λ��
		return File_Position + i;														
			//���ڻ������ڴ����ޣ�����������ͨ�����ļ��ֽ���һ����BufferȻ����ң���ɲ�������Buffer����λ��Ϊ1��ѭ��
			//ÿ��ѭ����������û�ҵ�Ŀ��λ�������BufferȻ������ļ��к�����ֽ����ݣ�������ͣѭ��Ѱ�ң�ֱ���ҵ�Ŀ������
			//������󷵻ص�ֵΪ��ǰѭ����Ŀ��λ��i����ǰ���Ѿ����������ѭ�����ֽ���Ŀ��
	else
		return 0;																		//��û�ҵ����򷵻�0��
}
//��������ļ��в����PPI�����ļ���ť��
void Button_BrowsPPI()
{
	LPTSTR Size_File;																	//��LPTSTR����Ŀ���ļ�
	OsdSetFilter(TEXT("�ı��ļ�(*.txt)|*.txt|�����ļ�(*.dat)|*.dat"), true);
																						//Ѱ�ҵı����ļ��ĸ�ʽ������׺����
	Size_File = OsdOpenDlg(form1.hWnd(), TEXT("��ѡ��PPI�����ļ�"));			//ѡ�񱾵��ļ���ѡ���Ĵ�������
	if (*Size_File)																		//����ѡ����Ч���ҵ�һ���ַ�Ϊ�ǿյ��ļ�	
	{
		form1.Control(IDC_COMB_PPIFiles).AddItem(Size_File);			//����ѡ����ļ�ȫ����ӵ���Ͽ�COMB_PPIFiles
		form1.Control(IDC_COMB_PPIFiles).TextSet(Size_File);			//����Ͽ�COMB_PPIFiles����ʾ��ѡ�ļ���ȫ·��
	}
	return;
}
//ͨ������϶�ΪPPIFiles�б������ļ�������
void COMBPPIFiles_FilesDrop(int ptrArrayFiles, int PPI_Cnt, int x, int y)
{
	TCHAR** Files = (TCHAR**)ptrArrayFiles;
	//��ptrArrFilesת��ΪTCHAR ** ���ͣ�����ά���飬��������ʹfiles������ݱ��浽��ά������
	//Ҳ�����ַ�����һά���飬ͨ����file[i]������ȡ��file�ļ��еĸ����ַ���
	//��Щ�ַ��������û����϶����ļ��У����϶��ؼ��ϵĸ����ļ���
	form1.Control(IDC_COMB_PPIFiles).AddItem(Files[1]);
	form1.Control(IDC_COMB_PPIFiles).TextSet(Files[1]);
	//�˴�ֻ��һ���ļ�file[1]��������Ӳ���ʾ����Ͽ�
	return;
}
/*
Ϊ��ȡ�����ݿⰴť���蹦�ܣ�
1.�������ȡ�����ݿ⡱��ť����ȡ�����ݿ⣬Ȼ��ť��Ϊ��ȡ����
2.�����ȡ������ťʱ��ֹ��ȡ������Ȼ��ť��Ϊ����ȡ�����ݿ⡱��
*/
void Button_ExtractSpeciesID()
{
	//�·�ifѭ��Ϊÿ�κ������ȼ���Ƿ�Ϊ��ȡ��״̬�������ǣ���ִ���·�ѭ��
	if (_tcscmp(form1.Control(IDC_Button_ExtractSpeciesID).Text(), TEXT("ȡ��")) == 0) 
	{
		if (MsgBox(TEXT("��ȡ��δ���,ȷ��Ҫȡ����"), TEXT("ȡ������"), mb_YesNoCancel, mb_IconQuestion) != idYes)
			return;
		//���ȡ����ť֮�󣬵����Ի���ѯ���û��Ƿ����Ҫȡ�������Է�ֹ��
		//���û�ѡ���ǡ�������ȫ��bool����IfCancel_Sign��Ϊtrue����ʾ�û�ѡ���ж���ȡ������
		IfCancel_Sign = true;
		return;
	}
	LPTSTR Size_File;
	Size_File = form1.Control(IDC_COMB_PPIFiles).Text();						//��LPTSTR�����洢�б�IDC_COMB_PPIFiles������
	if (*Size_File == 0)																//���б�Ϊ�գ��򱨴��Ƴ�
	{
		MsgBox(TEXT("��ѡ��PPI�����ļ���"), TEXT("δѡ���ļ�"), mb_OK, mb_IconExclamation);
		return;
	}

	int RollsNumber = form1.Control(IDC_COMB_SpeciesIDs).ListIndex();			//ȡ���б�ǰ����
	int TaxIDs = form1.Control(IDC_COMB_SpeciesIDs).ItemData(RollsNumber);
																						//ȡ�����������ID
	if (TaxIDs <= 0)																	//��������ID�򱨴��˳�
	{
		MsgBox(TEXT("��ѡ������ID��"), TEXT("δѡ������ID"), mb_OK, mb_IconExclamation);
		return;
	}
	HANDLE Open_Files;																	
	Open_Files = EFOpen(Size_File);												//�����ʧ���򷵻�ֵΪINVALID_HANDLE_VALUE
	if (Open_Files == INVALID_HANDLE_VALUE)												//����ʧ���򱨴��˳�
	{
		MsgBox(TEXT("��PPI�����ļ�ʧ�ܣ�"), TEXT("���ļ�ʧ�ܣ�����֤·�������ԣ�"), mb_OK, mb_IconExclamation);
		return;
	}
	IfCancel_Sign = false;																//���û�����ȡ����־����Ϊfalse
	form1.Control(IDC_Button_ExtractSpeciesID).TextSet(TEXT("ȡ��"));	//ִ����ȡ���ܺ󽫰�ť��ʾ��Ϊ��ȡ����

	tstring String_Find;																//����Ŀ���ַ���
	String_Find = Str(TaxIDs);							
	String_Find += TEXT(".ENSP");														//ƴ����9606.ENSP��
	LONGLONG Aim_Position = FindPos_PPIFile(Open_Files,String_Find.c_str());	//��λ��Ŀ��StringFind���ڵ��ڴ�λ��
																						//FindPos_PPIFile���ݲ��ֵĽ�����鿴�ú����Ķ���
	if (Aim_Position == 0)																//����ֵΪ0�����û�ҵ�	
	{
		form1.Control(IDC_Static_Status).TextSet(TEXT("���ļ���δ�ҵ� : "));
		form1.Control(IDC_Static_Status).TextAdd(String_Find);
																						//��ʾ�û�δ�ҵ�Ŀ��
	}
	else
	{
		//���ҵ�Ŀ�꣬�����״̬
		form1.Control(IDC_Static_Status).TextSet(TEXT("���ļ������ҵ�"));
		form1.Control(IDC_Static_Status).TextAdd(String_Find);
		form1.Control(IDC_Static_Status).TextAdd(TEXT(" �Ŀ�ʼλ��Ϊ ��"));
		form1.Control(IDC_Static_Status).TextAdd((double)Aim_Position);
	}
	form1.Control(IDC_Static_Status).TextAdd(TEXT("��"));			
	EFClose(Open_Files);															//�ر��ļ�
	if (Aim_Position)																	
		ReadPPIToDB(Size_File, Aim_Position, String_Find.c_str());
	//���ҵ�Ŀ�꣬��ͨ��ReadPPIToDB��������Aim_Poisiton�п�ʼһֻ��ȡ�����ݲ�ΪString_Find����
	//����ȡ�õ���ÿ����Ϣ�������ݿ��ProtLinks����
	form1.Control(IDC_Button_ExtractSpeciesID).TextSet(TEXT("��ȡ�����ݿ�"));
	//�����ȡ�󣬽���ť��ʾ�����ء���ȡ�����ݿ⡱
	return;
}
//��Ŀ�����ݿ������е�����ID����ʾ��IDC_COMB_SpeciesIDs�б���
void ShowSpeciesID()
{
	CBAdoRecordset rs;																	//��CBAdoRecordeset������װADO�е�RecordSet����
	tstring Items_Total;																//�ܵ�����ID�ַ���
	int SpeciesID = 0, RollsNumber = 0;													//��ʼ��������ID���б���Ŀ

	if (!rs.Open(TEXT("SELECT * FROM taxes")))										//ʹ��SQL��������ݿ��в��ң����������ݿ�ʧ���򷵻�ֵΪ0�����ɹ��򷵻�ֵΪ1
	{
		MsgBox(ADOConn.ErrorLastStr(), TEXT("��ȡ������Ϣʧ�ܣ�"), mb_OK, mb_IconExclamation);
																						//�������ݿ�ʧ�ܣ����û�����
		return;
	}

	form1.Control(IDC_COMB_SpeciesIDs).ListClear();							//���IDC_COMB_SpeciesIDs�б�������

	while (!rs.EOFRs())																	//����Ƿ��ȡ�꣬����ȡ��ȫ������ID����ֹѭ�����������ѭ��
	{
		Items_Total = rs.GetField(TEXT("TaxID"));								//ȡTaxID�еĵ�ǰ�е�������Ϊ��i������ID����
		SpeciesID = (int)Val(Items_Total.c_str());								//������ոջ�ȡ���ַ�������ת��Ϊ���α���
		Items_Total = TEXT("(") + Items_Total;											//���������ַ���
		Items_Total = rs.GetField(TEXT("Organism")) + Items_Total + TEXT(")");	//��ȡOrganism�еĵ�ǰ���������ݼ������ַ�����
		RollsNumber = form1.Control(IDC_COMB_SpeciesIDs).AddItem(Items_Total);			//����ת���õ������ַ�����Ŀ������ID�����䱣�浽
																						//IDC_COMB_SpeciesIDs�б��У����ҰѸñ�ǰ���Ѵ��������ص�RollsNumber��
		form1.Control(IDC_COMB_SpeciesIDs).ItemDataSet(RollsNumber, SpeciesID);			//�ڵ�RollsNumber����ʾ�ղű��������ID
		
		rs.MoveNext();																	//������ȡ����Ŀ���ƶ�����һ�С�
	}
	rs.Close();																			//�رռ�¼��
	if (form1.Control(IDC_COMB_SpeciesIDs).ListCount() > 0)					//���б�Ϊ�գ���ֱ�Ӷ�λ��������Ҫ��HomoSpeicies��һ�С�
		form1.Control(IDC_COMB_SpeciesIDs).ListIndexSet(488);
	return;
}
//Dijkstra����ʱ�Ľ��ȷ���������
bool DijkstraCalculatingCallBack(int Cycled_Times, int TotalCycleCurrent_Number, long userData)
{
	CBControl Static_Status = form1.Control(IDC_Static_Status);				//��CBcontrol����״̬��ʾ�ı�����󣬷�����������д����
	/////////////////////////////////////////////////////////////
	//��ʾ��ǰѭ�����ȡ�
	Static_Status.TextSet(Cycled_Times);											
	Static_Status.TextAdd(TEXT(" ��ѭ�������"));

	if (TotalCycleCurrent_Number)														//������ʣ��ѭ��δ��ɣ�����ʾʣ�����
	{
		Static_Status.TextAdd(TEXT("��ʣ��"));
		Static_Status.TextAdd(TotalCycleCurrent_Number);
		Static_Status.TextAdd(TEXT(" ��ѭ������"));
	}
	else
		Static_Status.TextAdd(TEXT("��"));
	//��ʾ��ǰѭ�����ȡ�
	/////////////////////////////////////////////////////////////
	DoEvents();																			//�ó�CPU����û��Ƿ�ѡ���ж�

	return(!IfCancel_Sign);																//�����û��Ƿ�ѡ���ж�
}
//�����ݿ�PathTasks���л������������·����Ȼ�󱣴浽BetwResults���еĺ�����
void Button_KeepAnalysing()
{
	CBAdoRecordset rsTasks;																////��CBAdoRecordeset������װADO�е�RecordSet����
	CBDijkstra DJMap;																	//����DiJkstra�㷨����
	int TasksTotal_Count = 0, TasksFinish_Count = 0;									//��������������ɵ�������
	long ID_Begin = 0, ID_End = 0, Final_Distance = 0;									//��㡢�յ㡢���·��
	CBControl Static_Count(IDC_Static_Count);											
	CBControl Static_Doing(IDC_Static_Doing);
	CBControl Static_Status(IDC_Static_Status);
	//��CBControl��������״̬��ʾ���������������form1.Control(******)����
	//������������д�����ġ�
	IfCancel_Sign = false;																//����û�δ����ȡ��������
	rsTasks.Open(TEXT("SELECT COUNT(*) FROM PathTasks WHERE DoneState=0"));
	//ʹ��SQL��������ݿ��в��ң����������ݿ�ʧ���򷵻�ֵΪ0�����ɹ��򷵻�ֵΪ1
	//��DoneState=0��λ�ÿ�ʼ�������·�����������֮���Ǹ��е�DoneState=1
	TasksTotal_Count = (int)Val(rsTasks.GetField((long)0));				//��ȡ����������
	rsTasks.Close();																	//�رռ�¼��
	if (TasksTotal_Count <= 0)															//����������С�ڵ���0��������������������ȫ�����
	{
		Static_Count.TextSet(TEXT(""));
		Static_Doing.TextSet(TEXT(""));
		Static_Status.TextSet(TEXT("PathTasks ���������񣬻���������ɡ�"));
		//״̬��ʾ����ʾ��ǰ״̬����ʾ�û��޿ɼ��������񣬲��˳����㡣
		return;
	}
	
	form1.Control(IDC_Button_Analysis).TextSet(TEXT("�жϷ���"));		//��������֮�󽫷�����ť��Ϊ�жϷ�����ť
	form1.Control(IDC_Button_KeepAnalysing).EnabledSet(false);		//������������ť��Ϊ���ɻ���
	form1.Control(IDC_Button_PreViewResult).EnabledSet(false);		//���鿴���������ť��Ϊ���ɻ���
	//���ϲ������Է�ֹ�û�����������֮����������ť���³������
	Static_Count.TextSet(TEXT("���������� "));									
	Static_Count.TextAdd(TasksTotal_Count);
	Static_Doing.TextSet(TEXT(""));
	Static_Status.TextSet(TEXT(""));
	Static_Status.TextSet(TEXT("���ڹ������硭��"));								
	//��ʾĿǰ�ķ�������״̬
	rsTasks.Open(TEXT("SELECT COUNT(*) FROM ProtLinks"));							//ʹ��SQL��������ݿ��в��ң����������ݿ�ʧ���򷵻�ֵΪ0�����ɹ��򷵻�ֵΪ1
	int LinksTotal_Count = (int)Val(rsTasks.GetField((long)0));			//ȡ���ܽ�����
	rsTasks.Close();																	//�ر����ݼ�
	form1.Control(IDC_PRO_Status).MaxSet(LinksTotal_Count);			
	form1.Control(IDC_PRO_Status).MinSet(0);
	form1.Control(IDC_PRO_Status).ValueSet(0);
	form1.Control(IDC_PRO_Status).VisibleSet(true);
	//ͨ����������ʾ��ǰ����
	DJMap.Clear();																		//���Dijkstra�㷨���е��ڽӾ��������洢�µĽڵ�ͱߵ����ݡ�
	if (!rsTasks.Open(TEXT("SELECT * FROM ProtLinks")))							//ʹ��SQL��������ݿ��в��ң����������ݿ�ʧ���򷵻�ֵΪ0�����ɹ��򷵻�ֵΪ1
	{
		Static_Status.TextSet(TEXT("��������ʧ�ܡ��������ݿ��ProtLinks����"));//���򿪱��ʧ�ܣ��򱨴��˳�
		return;
	}
	int Num_Cnt = 0;																	//�����洢Ŀǰ�Ѿ���ӵĹ�ϵ��Ŀ
	while (!rsTasks.EOFRs())															//��rsTasks�е�����δ��ȡ�꣬�����ѭ�������Ѷ�ȡ�����һ�У��򷵻�ֵΪ1������ֹѭ��
	{
		ID_Begin = (long)Val(rsTasks.GetField(TEXT("ENSPID1")));		//��ȡENSPID1�еĵ�ǰ��Ϊ���
		ID_End = (long)Val(rsTasks.GetField(TEXT("ENSPID2")));			//��ȡENSPID2�еĵ�ǰ��Ϊ�յ�
		Final_Distance = (long)Val(rsTasks.GetField(TEXT("Distance")));
																						//��ȡ��ǰ�������׼��·��Ϊ���·���������㽫���¸�����
		DJMap.AddNodesDist(ID_Begin, ID_End, Final_Distance);				//������ȡ�õ���㡢�յ㡢�������Dijkstra�ĵ�ͼ��
		Num_Cnt++;																		//���һ����ϵ�����벢��¼
		form1.Control(IDC_PRO_Status).ValueSet(Num_Cnt);				//����ǰ��ɵ���������ʾ����������
		if (Num_Cnt % 599 == 0)
		{
			//ÿ599����ϵ�����ˢ��һ��״̬��ʾ�ı��򣬲��鿴�û��Ƿ�����ȡ����
			Static_Doing.TextSet(ID_Begin);
			Static_Doing.TextAdd(TEXT(" -- "));
			Static_Doing.TextAdd(ID_End);
			Static_Doing.TextAdd(TEXT("  "));
			Static_Doing.TextAdd(Final_Distance);
			Static_Count.TextSet(Num_Cnt);
			Static_Count.TextAdd(TEXT(" / "));
			Static_Count.TextAdd(LinksTotal_Count);
			DoEvents();																	//�ó�CPU���
			if (IfCancel_Sign)
			{
				//���û�����ȡ������������ʾ�û��жϲ��˳�ѭ����
				Static_Status.TextSet(TEXT("�û��жϡ�"));						
				break;
			}
		}
		rsTasks.MoveNext();																//�������ݴ�����֮���ƶ�����һ�С�
	}
	rsTasks.Close();																	//�ر����ݼ�
	if (!IfCancel_Sign)	
	{
		//���û�û��ȡ�������������繹����ɣ�����ʾ��ǰ״̬
		Static_Status.TextSet(TEXT("����������ɡ�"));
		Static_Count.TextSet(LinksTotal_Count);
		Static_Doing.SelTextSet(TEXT(""));
	}
	else
	{
		//���û�ȡ���������򽫸�����ť�Ļس�ʼ״̬���˳�����
		form1.Control(IDC_Button_Analysis).TextSet(TEXT("����"));		//����ȡ����������ť�Ļء���������ť	
		form1.Control(IDC_Button_KeepAnalysing).EnabledSet(true);	//����������������ť����Ϊ�ɻ���
		form1.Control(IDC_Button_PreViewResult).EnabledSet(true);	//�����鿴�����������ť����Ϊ�ɻ���
		form1.Control(IDC_PRO_Status).VisibleSet(false);			//������������Ϊ�����ӣ������ؽ�����
		return;
	}
	////////////////////////////////////////////////////////////////
	//�Ϸ����ڴ洢��ʼ��ͼ������Ϣ���·�������ɷ��������·������//
	////////////////////////////////////////////////////////////////
	long* FinalPathNodes;																//���ڱ����������·��
	long PathNode_Number=0;																//���·�����������Ľ�����
	CBHashLK Hash_BetwKey;																//�ù�ϣ����[��Ŧ���ID�ļ�,������ֵ]
	CBAdoRecordset rsBetw;																//��CBAdoRecordeset�����ӿڣ�������BetwResults���е�ADO RecordSet����	

	form1.Control(IDC_PRO_Status).MaxSet(TasksTotal_Count);			
	form1.Control(IDC_PRO_Status).MinSet(0);
	form1.Control(IDC_PRO_Status).ValueSet(0);
	form1.Control(IDC_PRO_Status).VisibleSet(true);
	//��������ʾ��ǰ�������·���Ľ��ȡ�
	rsTasks.Open(TEXT("SELECT * FROM BetwResults"));								//ʹ��SQL��������ݿ��в��ң����������ݿ�ʧ���򷵻�ֵΪ0�����ɹ��򷵻�ֵΪ1
	while (!rsTasks.EOFRs())															//һֱ��ȡ�����һ�У����Ѷ������һ���򷵻�1
	{
		Hash_BetwKey.Add((long)Val(rsTasks.GetField(TEXT("Betw"))), (long)Val(rsTasks.GetField(TEXT("ENSPID"))), 0, 0, 0, 0, 0, false);
		//��Betw�е�ǰ�е����ݣ���ENSPID�е�ǰ�е����ݴ����ϣ���У��������������
		rsTasks.MoveNext();																//��ǰ�б����꣬�ƶ�����һ��
	}
	rsTasks.Close();																	//�ر����ݼ�

	rsTasks.Open(TEXT("SELECT * FROM PathTasks WHERE DoneState=0 ORDER BY ENSPID1, ENSPID2") );
	//ʹ��SQL��������ݿ��в��ң����������ݿ�ʧ���򷵻�ֵΪ0�����ɹ��򷵻�ֵΪ1
	//��ѯ���ݿ��ø������񣬰�ENSPID1��˳���������������ͬ�������·������
	//������Wiֻ�����һ�Σ��Ӷ���ʡ�˼���ʱ�䡣
	//����ʱ��Ϊ[O(nlogn)��O(n2)]������ʱ�����ʡ��O(n2)
	while (!rsTasks.EOFRs())															//һֱ��ȡ�����һ�У����Ѷ������һ���򷵻�1
	{
		ID_Begin = (long)Val(rsTasks.GetField(TEXT("ENSPID1")));		//ȡENSPID1�е�ǰ�е�������Ϊ���
		ID_End = (long)Val(rsTasks.GetField(TEXT("ENSPID2")));			//ȡENSPID2�е�ǰ�е�������Ϊ�յ�
		if (!DJMap.NodeIndex(ID_Begin) || !DJMap.NodeIndex(ID_End))			//����㲻���ڻ��յ㲻��������������
		{
			rsTasks.MoveNext();
			continue;
		}
		//��ʾ��ǰ�ļ���״̬�����ȡ�
		Static_Doing.TextSet(TEXT("���ڼ������·����"));							
		Static_Doing.TextAdd(ID_Begin);
		Static_Doing.TextAdd(TEXT(" -- "));
		Static_Doing.TextAdd(ID_End);

		Static_Count.TextSet(TasksFinish_Count);
		Static_Count.TextAdd(TEXT(" / "));
		Static_Count.TextAdd(TasksTotal_Count);
		//��ʾ��ǰ�ļ���״̬�����ȡ�
		PathNode_Number = DJMap.GetDistance(ID_Begin, ID_End, Final_Distance, FinalPathNodes, DijkstraCalculatingCallBack);
		//����ȡ�����������·��
		Static_Status.TextSet(TEXT("���·����������ɣ�"));						//��ʾ��ǰ�����������

		if (PathNode_Number > 0)														//���������������0
		{
			Static_Status.TextSet(TEXT("·���н����="));							//��ʾ·���н����
			Static_Status.TextAdd(PathNode_Number);
			for (int i = 1; i < PathNode_Number - 1; i++)								//��ȥi=0��i=pathnumber-1��������ʼ�ڵ�
			{
				if (Hash_BetwKey.IsKeyExist(FinalPathNodes[i]))						//����ϣ�����Ѵ��ڴ˽�㣬����ֵ+1
					Hash_BetwKey.ItemSet(FinalPathNodes[i], Hash_BetwKey.Item(FinalPathNodes[i], false) + 1);
				else
					Hash_BetwKey.Add(1, FinalPathNodes[i]);						//����ϣ����δ��¼�˽�㣬����Ӵ˽�㲢����ֵΪ1
			}
		}
		else                                                                            //�����������С�ڵ���0
			if (PathNode_Number < 0)													//��С��0������ʾ���������·��
				Static_Status.TextSet(TEXT("��·����"));			
			else
			{
				Static_Status.TextSet(TEXT("�������"));						//������0�����ʾ�����������DJ����ļ��������Ϣ�ṩ���û�
				Static_Status.TextAdd(DJMap.ErrDescription);
			}

		rsTasks.SetField(TEXT("DoneState"), 1);						//��ɵ�ǰ���񣬽���DoneState���Ϊ1
		TasksFinish_Count++;															//����ɵ�������+1
		form1.Control(IDC_PRO_Status).ValueSet(TasksFinish_Count);	//��������ʾ��ǰ����
		DoEvents();																		//�ó�CPU����Ӧ�û��Ƿ�ѡ���ж�
		if (IfCancel_Sign)																//���û�ѡ���жϣ�����ֹѭ��
			break;

		rsTasks.MoveNext();																//�������ݴ����꣬�ƶ�����һ��
	}

	Static_Status.TextSet(TEXT("�������ݿ�BetwResults��ɾ�������ݡ���"));			//��ʾ��ǰ�Ĳ���
	ADOConn.Execute(TEXT("DELETE FROM BetwResults"));								//ͨ��ADOConn�����ݿ����SQL�������ò���Ϊɾ��BetwResults��
	Static_Status.TextSet(TEXT("�������ݿ�BetwResults�����������ݡ���"));			//��ʾ��ǰ����
	rsBetw.Open(TEXT("SELECT * FROM BetwResults"));								//ʹ��SQL��������ݿ��в��ң����������ݿ�ʧ���򷵻�ֵΪ0�����ɹ��򷵻�ֵΪ1		
	for (int i = 1; i <= Hash_BetwKey.Count(); i++)
	{
		//����BetwResults���е�����
		rsBetw.AddNew();																//��ʼ����һ������
		rsBetw.SetField(TEXT("ENSPID"), Hash_BetwKey.IndexToKey(i));
		//��ENSPID�е�ǰ�е�ֵ����Ϊ����ϣ���еļ���
		rsBetw.SetField(TEXT("Betw"), Hash_BetwKey.ItemFromIndex(i, false));
		//��Betw�е�ǰ�е�ֵ����Ϊ����ϣ��μ���Ӧ��ֵ��
		rsBetw.Update();																//�������²�����
	}
	rsBetw.Close();																		//�ر����ݼ�
	Static_Status.TextSet(TEXT("�������ݿ� BetwResults ��ɡ�"));					//��ʾ��ǰ����

	rsTasks.Update();																	//����rsTasks�еĸղŸ���ӽ���������Ϊ��1�������ݣ����������ݿ⡣
	rsTasks.Close();																	//�ر����ݼ�
	
	if (!IfCancel_Sign)																	//���û�δѡ��ȡ���ж�
	{
		Static_Count.TextSet(TEXT("����������"));
		Static_Count.TextAdd(TasksTotal_Count);
		Static_Doing.TextSet(TEXT(""));
		Static_Status.TextSet(TEXT("������������ɡ�"));
		//��ʾ��ǰ����״̬����ʾ�û��������������
	}
	else
		Static_Status.TextSet(TEXT("�û��жϡ�"));								//���û�ѡ���жϣ�����ʾ�û��ж�
	//�˴����û�Ҫô�Ѿ������ȫ����������Ҫôѡ�����ж�
	//�����������������Ҫ�ָ�������ť״̬
	form1.Control(IDC_Button_Analysis).TextSet(TEXT("����"));			
	form1.Control(IDC_Button_KeepAnalysing).EnabledSet(true);
	form1.Control(IDC_Button_PreViewResult).EnabledSet(true);
	form1.Control(IDC_PRO_Status).VisibleSet(false);
	return;
}
//����������ť�Ĺ��ܺ������ֱ��д�ð�ť����״̬�µĹ��ܡ�
void Button_Analysis()
{	
	//===================================//
	//����ʱ��ťΪ���жϷ�����״̬��
	if (_tcscmp(form1.Control(IDC_Button_Analysis).Text(), TEXT("�жϷ���")) == 0)
	{
		//��ѯ���û��Ƿ�Ҫ�����жϲ��������û�ѡ���ǣ����жϱ������Ϊtrue���������������жϲ����������Ƴ���ǰ���ܡ�
		if (MsgBox(TEXT("������δ���,ȷ��Ҫȡ����\r\n����ɵķ�������������ݿ��б��档\n�жϺ��������´μ������з�����"), TEXT("�жϷ���"), mb_YesNoCancel, mb_IconQuestion) == idYes)
			IfCancel_Sign = true;
		return;
	}
	//����ʱ��ťΪ���жϷ�����״̬��
	//===================================//
	//����ʱ��ťΪ��������״̬��
	//ѯ���û��Ƿ�Ҫ��ͷ�������ݣ�������Ѿ��������������Ϣ��
	if (MsgBox(TEXT("�˲��������¶�ȡ ENSP���ݼ��ļ������ɺ��ؽ����ݿ�Ĵ����������б���ǰ�ķ�������������Ƿ������"), TEXT("ȷ�����·���"), mb_OkCancel, mb_IconQuestion) != idOk)
		return;
	
	LPTSTR Size_File = form1.Control(IDC_COMB_EnspLisFiles).Text();			//�����б�IDC_COMB_EnspLisFiles��������Ϣ
	CBControl Static_Count(IDC_Static_Count);
	CBControl Static_Doing(IDC_Static_Doing);
	CBControl Static_Status(IDC_Static_Status);
	//��CBControl��������״̬��ʾ���������������form1.Control(******)����
	//������������д�����ġ�
	if (*Size_File == 0)																//���б�IDC_COMB_EnspLisFilesΪ�գ��򱨴��˳�
	{
		MsgBox(TEXT("��ѡ��ENSP���ݼ��ļ���"), TEXT("δѡ���ļ�"), mb_OK, mb_IconExclamation);
		return;
	}
	Static_Status.TextSet(TEXT("��� TarProts ����"));							//��ʾ��ǰ����
	ADOConn.Execute(TEXT("DELETE FROM TarProts"));									//���TarProts��

	Static_Status.TextSet(TEXT("��ȡ ENSP���ݼ��ļ�����"));						//��ʾ�������еĲ���
	
	CBReadLinesEx Aiming_File;															//��CBReadLinesEx�����洢Ŀ���ļ�
	CBAdoRecordset rs;																	
	LPTSTR Size_Line;																	
	tstring String_Field;																//�洢Ŀ���ַ���
	LPTSTR String_SQL = NULL;															//�洢SQL���
	if (!rs.Open(TEXT("SELECT * FROM TarProts")))									//ʹ��SQL��������ݿ��в��ң����������ݿ�ʧ���򷵻�ֵΪ0�����ɹ��򷵻�ֵΪ1
		return;
	if (!Aiming_File.OpenFile(Size_File))										//��Ŀ���ļ�����Ŀ���ļ���ʧ�ܣ����˳�
		return;
	while(!Aiming_File.IsEndRead())														//��Ŀ���ļ�δ��ȡ�����һ�������ѭ��
	{
		Aiming_File.GetNextLine(Size_Line);										//��ȡ��һ���ַ���
		if (Aiming_File.IsErrOccured())													//���������˳��ò���
			return;

		String_Field = Size_Line;														//��Size_Line��ֵ��Field����Field�ж�ǰ4���ַ��Ƿ�Ϸ�
		if (String_Field.substr(0, 4) == TEXT("ENSP"))						//��ǰ�ĸ��ַ��ǡ�ENSP������ִ���������
		{
			rs.AddNew();																//ΪTarProts���������
			rs.SetField((long)0, Size_Line + 4);							//��ǰ4���ַ�ȥ������ɾ����ENSP������뵽TarProts����
			rs.Update();																//������������²���
		}
	}
	rs.Close();																			//�ر����ݼ�
	Static_Status.TextSet(TEXT("���ݿ� TarProts ������ɡ�"));					//��ʾ��ǰ״̬

	ADOConn.Execute(TEXT("DROP TABLE PathTasks"));									//�����д˱����ɾ�����еı��
	String_SQL= TEXT("SELECT T1.ENSPID AS ENSPID1, T2.ENSPID AS ENSPID2, 0 AS DoneState INTO PathTasks FROM TarProts T1, TarProts T2 WHERE T1.ENSPID < T2.ENSPID GROUP BY T1.ENSPID, T2.ENSPID");
	//�������·�������PathTasks
	//�����У��ֱ�Ϊ��㡢�յ㡢��̾���
	ADOConn.Execute(String_SQL);
	//ִ��SQL���
	bool If_PathTasksRun=true;															//���PathTasks�����ɳɹ�
	if (!rs.Open(TEXT("SELECT * FROM PathTasks")))									//ʹ��SQL��������ݿ��в��ң����������ݿ�ʧ���򷵻�ֵΪ0�����ɹ��򷵻�ֵΪ1
		If_PathTasksRun = false;														//��ѯ��ʧ�ܣ������PathTasks������ʧ��
	else
		if (rs.EOFRs())																	//�����Ϊ�գ�Ҳ��ʾ����ʧ��
			If_PathTasksRun = false;
	if (!If_PathTasksRun)																//������ʧ�ܣ��򱨴��˳�
	{
		MsgBox(TEXT("PathTasks �����ɿ���ʧ�ܣ�\n���ڴ˴����öϵ㣬Ȼ�󽫱���String_SQL�����ݴӼ��Ӵ����и���ճ����Access�Ĳ�ѯ�У���Acess�����д˲�ѯ��顭��"));
		return;
	}

	ADOConn.Execute(TEXT("DELETE FROM BetwResults"));								//���BetwResults��
	Button_KeepAnalysing();																//������׼���ã�ִ�з�������
	return;
}
//��������ļ��в����ENSP�ļ���ť��
void Button_BrowsENSP()
{
	//�˴�������Button_BrowsPPI�����������ݻ�����ͬ���ʲ��ٹ���ע��
	LPTSTR Size_File;
	OsdSetFilter(TEXT("�ı��ļ�(*.txt)|*.txt|�����ļ�(*.dat)|*.dat"), true);
	Size_File = OsdOpenDlg(form1.hWnd(), TEXT("��ѡ��ENSP�����б��ļ�"));
	if (*Size_File)
	{
		form1.Control(IDC_COMB_EnspLisFiles).AddItem(Size_File);
		form1.Control(IDC_COMB_EnspLisFiles).TextSet(Size_File);
	}
	return;
}
//ͨ������϶�ΪEnspLisFiles�б������ļ�������
void COMBEnspLisFiles_FilesDrop(int ptrArrayFiles, int ENSP_Cnt, int x, int y)
{
	TCHAR** Files = (TCHAR**)ptrArrayFiles;
	//��ptrArrFilesת��ΪTCHAR ** ���ͣ�����ά���飬��������ʹfiles������ݱ��浽��ά������
	//Ҳ�����ַ�����һά���飬ͨ����file[i]������ȡ��file�ļ��еĸ����ַ���
	//��Щ�ַ��������û����϶����ļ��У����϶��ؼ��ϵĸ����ļ���
	form1.Control(IDC_COMB_EnspLisFiles).AddItem(Files[1]);
	form1.Control(IDC_COMB_EnspLisFiles).TextSet(Files[1]);
	//�˴�ֻ��һ���ļ�file[1]��������Ӳ���ʾ����Ͽ�
	return;
}
//�ͷ��ڴ沢�˳�������ĺ�����
void Button_ShutOff()
{
	form1.UnLoad();																		//�ͷų����ڴ�
	return;
}

int main()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//�����ؼ��Ĺ��ܺ�������ؼ���ع��������ӡ�
	form1.EventAdd(IDC_Button_Brows, eCommandButton_Click, Button_BrowsPPI);
	form1.EventAdd(IDC_Button_ExtractSpeciesID, eCommandButton_Click, Button_ExtractSpeciesID);
	form1.EventAdd(IDC_COMB_PPIFiles, eFilesDrop, COMBPPIFiles_FilesDrop);
	form1.EventAdd(IDC_COMB_EnspLisFiles, eFilesDrop, COMBEnspLisFiles_FilesDrop);
	form1.EventAdd(IDC_Button_Analysis, eCommandButton_Click, Button_Analysis);
	form1.EventAdd(IDC_Button_KeepAnalysing, eCommandButton_Click, Button_KeepAnalysing);
	form1.EventAdd(IDC_Button_BrowsENSP, eCommandButton_Click, Button_BrowsENSP);
	form1.EventAdd(IDC_Button_PreViewResult, eCommandButton_Click, Button_PreViewResult);
	form1.EventAdd(IDC_Button_ShutOff, eCommandButton_Click, Button_ShutOff);

	EventsMapFrmView();																	//���ؽ��Ԥ������ĸ��ؼ����书��
	//�����ؼ��Ĺ��ܺ�������ؼ���ع��������ӡ�
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	form1.IconSet(IDI_ICON1);


	if (!ADOConn.Open(TEXT("PPI.accdb")))
		MsgBox(ADOConn.ErrorLastStr(), TEXT("Sorry!main()���AD0Conn.Open()���ݿ�����ʧ�ܣ�"), mb_OK, mb_IconExclamation);
	
																	//��ӭ���棬����Ԥ�������	
	//Preset_Welcome();

	Button_PreViewResult();
	while (1)
	{
		if (BREAK_SIG)
			break;
		DoEvents();
	}

	form1.Show();
	ShowSpeciesID();																	//��Ŀ�����ݿ������е�����ID����ʾ��IDC_COMB_SpeciesIDs�б��

	return 0;
}