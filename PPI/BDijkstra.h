//////////////////////////////////////////////////////////////////////
// CBDijkstra�ࣺDijkstra �㷨������ͼ�����·����
//
// ֧�֣���Ҫ BWindows��BReadLinesEx ģ���֧�֣���Ҫ CBHashLK��CBArrLink �ࣩ
//////////////////////////////////////////////////////////////////////


#pragma once

#include "BWindows.h"

//////////////////////////////////////////////////////////////////////////
// About NODE ID and NODE Index:
// -----------------------------
// NODE ID is defined by users, the value can be any, and can be non-continuous. 
// NODE Index is continuous, from 1 to NodeCount. 
// NODE ID is primarily for user use; NODE Index is primarily for the
//   inner data structure use. 
//////////////////////////////////////////////////////////////////////////


// function pointer type pointing to a call back function for CBDijkstra.LoadFileData processing
// the function pointer can be specified in LoadFileData() call
//   if specified, LoadFileData() will call back the function when each several lines 
//   have been read
// parameters:
// fPercentRead:	file content percent (<=1.0) already read
// userData:		User defined data (specified in LoadFileData() call) 
// the call back function should returns true; if returns false, the LoadFileData() 
//   will beak reading file and exit immeadiately
typedef bool (*pFunDjReadFileCallBack)(float fPercentRead, long userData);


// function pointer type pointing to a call back function for CBDijkstra.GetDistance processing
// the function pointer can be specified in GetDistance() call
//   if specified, GetDistance() will call back the function when several cycles 
//   have been processed (Actually called back by Calculate())
// parameters:
// iCycled:	 how many cycles have been processed(incresing)
// iTotalCycleCurrent:  how many cycles left should be processed (may changing)
// userData:		User defined data (specified in GetDistance() call) 
// the call back function should returns true; if returns false, the GetDistance() 
//   will beak reading file and exit immeadiately
typedef bool (*pFunDjCalculatingCallBack)(int iCycled, int iTotalCycleCurrent, long userData);

class CBDijkstra
{
// ================ ���Ͷ��� ================ 
private:
	typedef struct _NodeInfType		// ����һ��������Ϣ
	{
		long ID;					// ��� ID���û�ʹ��ID����ֵ���Բ����������� index ��ͬ�����ߴ�1��ʼ����������
		CBArrLink *pEdges;			// ָ��һ�� CBArrLink �Ķ��󣬺��߱���ý�������ӵ����б�(��� index, ��Ȩֵ)
									//   pEdges->Item1() Ϊֱ�����ӵĽ�� index��
									//	 pEdges->Item2() Ϊֱ�����ӱߵ�Ȩֵ
									// �ڶ�ȡ�ļ�ʱ��Ҫ�ж���֧�ָýṹ�е����ݲ��ظ�
									//  �����������ļ�����������¼��1 2 dist��2 1 distʱ�ýṹ��Item1��Item2�����ظ���
 
		bool fVisited;				// �㷨������ʹ�õı�־����������ʱ�Ƿ񱻷������
		long w;						// �ý����ĿǰΪֹ�ġ���С·����ֵ��W
		CBArrLink *pParentList;		// �����С·������һ���ݸ���� index��ʹ���б���֧�ֲ������·��

		LPTSTR tag;					// ��㸽������
	} NodeInfType;


private:
	static const long mcMaxDistance;			// �����ֵ
	static const int mcNodesExpandStep;			// ��㶯̬�ռ�ÿ������Ľ�����

// ================ ���ݳ�Ա�ͺ�����Ա ================
public:
	bool ShowMsgIfFail;				// ʧ��ʱ�Ƿ��Զ�������ʾ
	TCHAR ErrDescription[1024];		// ������Ϣ

public:
	CBDijkstra(int ik=1);
	~CBDijkstra();
	
	// ��ȡһ�������ļ����ݲ����㣬�ɹ����ؽ������ʧ�ܷ��� 0
	// �����ļ�Ϊ�ı��ļ���û�б����У�ÿ����3�У������1ID  ���2ID  ����ֵ��
	//   �ָ���֧�ֿո�Tab�����š��ֺţ���Ҫȫ�ļ�ͳһʹ��һ�ַָ���
	// k=0 ʱʹ��ԭ�����ù��ı�����Ŀǰ�� k ֵ������ʹ�ñ������������� k ֵ
	// a user-defined call back function can be specified by pf, which the prototype is :
	//
	//       bool FunReadFileCallBack(float fPercentRead, long userData);
	//
	// if specified, LoadFileData will call this function back when each several lines 
	//   have been read. The function should returns true if the user want to continue. 
	//   If the function returns false, LoadFileData() will stop reading file and exit. 
	int LoadFileData(LPCTSTR szFileName, int ik=0, pFunDjReadFileCallBack pf = NULL, long userDataCallBack = 0);	
	
	// ���һ������ϵ������������ID���Լ����ǵľ���
	// �ɹ����� True��ʧ�ܷ��� False�������� ErrDescription��
	//   ��������������е���������ӡ���ϵ������ִ�� Clear() ����
	bool AddNodesDist(long idNode1, long idNode2, long distance);

	// ���һ���� idNodeStart �� idNodeEnd �����·��
	// �����������·���������Ľ�������������ʼ����ֹ��㣬>=2����ʧ�ܷ���0����·������-1
	// ���� pIdsInPath ָ��һ�����飨�±��0��ʼ�ã����Ӵ˲����������·�������������� id��
	//   ����ռ��ɱ����������� HM ����
	// �Ӳ��� distance �������·���ġ�Ȩֵ�͡�����·��ʱ���������ֵ mcMaxValue������ʱ����0��
	long GetDistance(long idNodeStart, long idNodeEnd, long &distance, long *&pIdsInPath, pFunDjCalculatingCallBack pf = NULL, long userDataCallBack = 0 );

	// ���ص�ǰ�Ѷ�������������н�����
	int NodeCount();

	// ����һ��Node index��������1--NodeCount()���������û��ĸý��� ID�����Բ�������
	long NodeID(int idxNode);

	// �������ID: idNode�����ظý���ڱ����ݽṹ�е� index��
	// �������ڸý�㷵�� 0 
	//�������ڲ���ĳ�� ID �Ľ���ڵ�ǰ�Ѷ�����������Ƿ���ڣ�
	int NodeIndex(long idNode);	

	// ���ص�ǰ�Ѷ���������У����㣨ID: idNode��ֱ�������ıߵ�����
	int NodeAdjEdgesCount(int idxNode);

	// �������� idxNode ֱ��������һ�����������ߵ�Ȩֵ
	// �Ӳ��� idxNodeAdj ����ֱ��������������� index
	// idxEdge ��Χ��1--NodeAdjEdgesCount(idxNode)
	long NodeOneAdjEdge(int idxNode, int idxEdge, int &idxNodeAdj);

	// ������ݽṹ�е���������
	void Clear();
	


private:
	int m_k;						// �Ƿ��� k-th ·����k>1����k=1ʱ�������·��

	NodeInfType *m_Nodes;			// ָ��һ�� NodeInfType ���͵����飬���汾ͼ�����н�㣬�Լ�������ӵ����б�
									//   �±��1��ʼ��0 �˷Ѳ��ã���Ч���ռ� m_Nodes[1]��m_Nodes[m_NodesCount]
									// Ԥ���ٵĿռ�Ϊ m_Nodes[0]��m_Nodes[m_NodesUbound]
									// m_Nodes[] �����±�ƽ�� index ����1��ʼ����������������� ID Ϊ�û�ID���Բ�����
									//  ID => index ת���� m_hashNodesIdxes��index => ID ת���� m_Nodes[index].ID
	int m_NodesCount,m_NodesUbound;	// ��ǰ����������ǰ m_Nodes ������±꣨m_Nodesָ��ռ��� m_NodesUbound+1 ���ռ䣩
	
	CBHashLK m_hashNodesIdxes;		// ��ϣ�������ɽ�� ID ת��Ϊ index��key = ID��Item = m_Nodes[] �����±�

	int m_indexStart;				// �� Calculate �����ô˱��������������������·���ġ���ʼ��� index��
	CBArrLink m_lkNodesTouch;		// �����·�������С��漰���Ľ�㡱�б���������δ visited �� w ���� �� �Ľ��� index
									// �����·��ʱ��ÿ�δ���ȡһ�� w ��С�Ľ����в�����
									//   ����˽���� visited
private:
	// ����һ����ʼ��㣬���㲢�����ݽṹ�б������·��״̬��
	//   �ֱ��¼������ i �����·����Ȩֵ�͡� m_Nodes[i].w
	//   ���ֱ��ǵ����� i �����·���Ļ��ݸ��ڵ� m_Nodes[i].pParentList
	// ֮��ֻҪ����һ��ĩβ��� i�����ܻ�õ��˽������·����Ȩֵ�͡��Լ����·��
	bool Calculate(long idNodeStart, pFunDjCalculatingCallBack pf=NULL, long userDataCallBack=0);

};