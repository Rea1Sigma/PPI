//////////////////////////////////////////////////////////////////////
// CBDijkstra ���ʵ�֣�������ͼ�����·��
//
//////////////////////////////////////////////////////////////////////

#include "BDijkstra.h"
#include "BReadLinesEx.h"


const long CBDijkstra::mcMaxDistance=999999999;
const int CBDijkstra::mcNodesExpandStep = 50000;

//////////////////////////////////////////////////////////////////////////
// ���������
//////////////////////////////////////////////////////////////////////////

CBDijkstra::CBDijkstra( int ik/*=1*/ )
{
	m_k = ik;	// k=1ʱ�������·��
	
	m_Nodes = NULL;
	m_NodesCount = 0;  m_NodesUbound=-1;
	
	m_indexStart=0;
	ShowMsgIfFail=true;
	memset(ErrDescription, 0, sizeof(ErrDescription));
}

CBDijkstra::~CBDijkstra()
{
	Clear();
}



//////////////////////////////////////////////////////////////////////////
// ���к���
//////////////////////////////////////////////////////////////////////////


int CBDijkstra::LoadFileData( LPCTSTR szFileName, int ik/*=0*/, pFunDjReadFileCallBack pf /*= NULL*/, long userDataCallBack /*= 0*/ )
{
	CBReadLinesEx dataFile;
	LPTSTR szLine=NULL, delimiter=NULL;
	TCHAR ** s=NULL;  
	int iFlds=0, i=0, j=0;
	bool blRetCallBack=false;

	if (ik>0) m_k = ik;

	if (! dataFile.OpenFile(szFileName)) 
	{
		_tcscpy(ErrDescription, TEXT("Open data file failed!"));  // "�������ļ�ʧ��"
		return 0;  // ���ļ�ʧ��
	}
	
	dataFile.TrimControls = true;
	dataFile.TrimSpaces = true;
	dataFile.IgnoreEmpty = true;
	dataFile.ShowMsgIfErrRead = ShowMsgIfFail;

	// ============ ׼�����ݽṹ ============
	// ���������������
	if (m_Nodes) Clear();

	// ============ ���ж�ȡ�ļ�����ȡ������������ ============
	long ii=0;

	// ��ʼ����һ�λص�����
	blRetCallBack=(*pf)(0, userDataCallBack);
	if (! blRetCallBack) 
	{ _tcscpy(ErrDescription, _TEXT("\nReading file not finished. Break by the user. ")); return 0;}

	// ��ȡ�ļ�
	while (! dataFile.IsEndRead())
	{
		dataFile.GetNextLine(szLine);
		if (dataFile.IsErrOccured()) 
		{
			_tcscpy(ErrDescription, TEXT("Error occurred when reading file. "));  // "��ȡ�����ļ�����"
			return 0;
		}
		
		// �����δȷ�����ֶηָ���������ȷ��
		if (delimiter==NULL)
		{
			if (InStr(szLine, TEXT(" ")))
				delimiter = TEXT(" ");
			else if (InStr(szLine, TEXT("\t")))
				delimiter = TEXT("\t");
			else if (InStr(szLine, TEXT(";")))
				delimiter = TEXT(";");
			else if (InStr(szLine, TEXT(",")))
				delimiter = TEXT(",");
			else
				delimiter = TEXT(" ");	// Ĭ�϶��ǿո�ָ�
		}
		
		// �ָ� szLine �е��ֶ�
		if (*szLine==0) continue;		// ����ǿմ�������
		iFlds=Split(szLine, s, delimiter);
		if (iFlds<3)
		{
			_tcscpy(ErrDescription, StrAppend(szLine, TEXT("\r\n"), 
				TEXT("Can not find 3 or more fields. Reading file aborted. ")));  // "�����ҵ�3�������ϵ��ֶΣ����ݶ�ȡ��ֹ��"
			if (ShowMsgIfFail) MsgBox(ErrDescription, 
				TEXT("Dijkstra data reading fail"), mb_OK, mb_IconExclamation);   // "Dijkstra���ݶ�ȡʧ��"
			return 0;
		}
		
		// ============ �������ֶ� ============
		if ( ! AddNodesDist( (long)Val(s[1]) , (long)Val(s[2]), (long)Val(s[3]) ) )
		{	_tcscpy(ErrDescription, StrAppend(szLine, TEXT("\r\n"), 
				TEXT("Can not save NODE "), TEXT(". "), TEXT("\r\n"), ErrDescription));    // "������ӽ�� "
			if (ShowMsgIfFail) MsgBox(ErrDescription, 
				TEXT("Dijkstra inner data structure error"), mb_OK, mb_IconExclamation); // "Dijkstra�ڲ����ݽṹ����"
			return  0;
		}

		ii++;
		if (pf && ii%5000==0)
		{
			blRetCallBack=(*pf)(dataFile.GetPercent(2), userDataCallBack);
			if (! blRetCallBack) 
			{	
				_tcscpy(ErrDescription, 
				  _TEXT("\nReading file not finished. Break by the user. ")); 
				return 0;	// break the reading file
			}		
		}

	}  // end while (! dataFile.IsEndRead())

	// ���ؽ������
	return m_NodesCount;
}



long CBDijkstra::GetDistance( long idNodeStart, long idNodeEnd, long &rDistance, long *&pIdsInPath, pFunDjCalculatingCallBack pf/*=NULL*/, long userDataCallBack /*= 0*/ )
{
	int iStart = 0, iEnd = 0;
	long *ids = NULL, iUboundIds=-1;		// ids ָ��һ�����飨�±��1��ʼ�ã�0�˷ѣ����������򱣴����·������� id
	int ctCount=0;							// ���·���ϵĽ����� 
	long iIdsExpandStep = 100;				// ids ����ÿ�������Ŀռ����
	long i = 0, j=0;

	// �ӽ�� id �����ʼ��� index => iStart
	iStart=m_hashNodesIdxes.Item(idNodeStart,false);
	if (iStart<=0) 
	{
		_tcscpy(ErrDescription, StrAppend(TEXT("Getting path failed: not found starting Node: "), Str(idNodeStart), TEXT(". ") ) );  // "���·��ʧ�ܣ�û���ҵ���ʼ��� "
		if (ShowMsgIfFail) MsgBox(ErrDescription, TEXT("Dijkstra Getting path failed"), mb_OK, mb_IconExclamation);
		return false;
	}
	
	// �ӽ�� id �����ֹ��� index => iEnd
	iEnd = m_hashNodesIdxes.Item(idNodeEnd, false);
	if (iEnd<=0) 
	{
		_tcscpy(ErrDescription, StrAppend(TEXT("Getting path failed: not found ending Node: "), Str(idNodeEnd), TEXT(". ") ) );   // "���·��ʧ�ܣ�û���ҵ���ֹ��� "
		if (ShowMsgIfFail) MsgBox(ErrDescription, TEXT("Dijkstra Getting path failed"), mb_OK, mb_IconExclamation);
		return false;
	}
	
	// �������·��״̬
	if ( m_indexStart<=0 || m_indexStart != iStart )		// ���������¼������·��״̬���������ݽṹ
		if (! Calculate(idNodeStart, pf, userDataCallBack) ) return 0;			// ����ʧ��

	// �ͷ� pIdsInPath ���ڴ棨����еĻ���
	if (HM.IsPtrManaged(pIdsInPath))
		HM.Free(pIdsInPath);

	// �� m_Nodes[iEnd] �������·�����
	i = iEnd;
	while (i != iStart && ctCount<m_NodesCount)
	{
		ctCount++;
		if (ctCount>iUboundIds)
			iUboundIds = Redim(ids, iUboundIds+iIdsExpandStep, iUboundIds, true);
		ids[ctCount] = m_Nodes[i].ID;

		// ��� m_Nodes[i] û�л��ݸ���㣬������·��
		if ( m_Nodes[i].pParentList->Count() == 0 )
		{
			rDistance = mcMaxDistance;	// �������ֵ
			delete []ids;
			pIdsInPath = NULL;
			return -1;					// ������·��
		}

		// �� m_Nodes[i] �Ļ��ݸ���� index => i
		i = m_Nodes[i].pParentList->Item(1);
		if ( i<=0 || i>m_NodesCount )		// �ݴ�
		{
			_tcscpy(ErrDescription, StrAppend(TEXT("Getting path failed: invalid parent of NODE (index="), Str(i) , TEXT("). ") ));   //"���·��ʧ�ܣ�·����;�Ļ��ݽ����Ч��index="
			if (ShowMsgIfFail) MsgBox(ErrDescription, TEXT("Dijkstra Getting path failed"), mb_OK, mb_IconExclamation);
			delete []ids;
			pIdsInPath = NULL;
			rDistance = 0;
			return false;
		}
	}	// end while (i != iStart && ctCount<m_NodesCount)

	if (i != iStart)	// �ݴ�
	{
		_tcscpy(ErrDescription, StrAppend(TEXT("Getting path failed: can not trace back to Starting NODE '"), Str(idNodeStart) , TEXT("'. ") ));  //"���·��ʧ�ܣ���·���в��ܻ��ݵ�����ʼ��� "
		if (ShowMsgIfFail) MsgBox(ErrDescription, TEXT("Dijkstra Getting path failed"), mb_OK, mb_IconExclamation);
		delete []ids;
		pIdsInPath = NULL;
		rDistance = 0;
		return false;
	}

	// ��·���еĸ���� id���� ids ������������� pIdsInPath��
	//  ��������ʼ��� id
	// ���������п��ٿռ䲢�� HM ����ռ�
	pIdsInPath = new long [ctCount+1];	// �±��0��ʼʹ�ã�+1��ʾ��һ����ʼ��� id
	HM.AddPtr(pIdsInPath);

	pIdsInPath[0] = idNodeStart;  j=1;	// �ֶ������ʼ���
	for(i=ctCount; i>0; i--)
		pIdsInPath[j++]=ids[i];

	// ����
	rDistance = m_Nodes[iEnd].w;
	delete []ids;
	return ctCount+1;
}



void CBDijkstra::Clear()
{
	int i;
	for (i=1; i<=m_NodesCount; i++)
	{
		if (m_Nodes[i].pEdges) 
		{ delete m_Nodes[i].pEdges; m_Nodes[i].pEdges=NULL; }
		
		if (m_Nodes[i].pParentList) 
		{ delete m_Nodes[i].pParentList; m_Nodes[i].pParentList=NULL; }
		
		if ( m_Nodes[i].tag ) 
		{ delete []m_Nodes[i].tag; m_Nodes[i].tag=NULL; }
	}

	if (m_Nodes) { delete []m_Nodes; m_Nodes=NULL; }
	m_NodesCount = 0;  m_NodesUbound=-1;

	m_hashNodesIdxes.Clear();
	m_hashNodesIdxes.AlloMem(mcNodesExpandStep);

	// ���ٵ�һ�� mcNodesExpandStep ���ռ�
	Redim(m_Nodes, mcNodesExpandStep, -1, false);
	m_NodesUbound=mcNodesExpandStep;

	m_indexStart = 0;					// ��ǰ m_Nodes[] ������¼�����·��״̬����ʼ�����Ч
	m_lkNodesTouch.Clear();
	memset(ErrDescription, 0, sizeof(ErrDescription));
}











//////////////////////////////////////////////////////////////////////////
// ˽�к���
//////////////////////////////////////////////////////////////////////////





bool CBDijkstra::Calculate(long idNodeStart, pFunDjCalculatingCallBack pf/*=NULL*/, long userDataCallBack/*=0*/)
{
	int ctVisits = 0;			// �� visited �����
	int iStart=0;				// ��Ӧ idNodeStart ����ʼ��� index
	int iVertex=0;				// ÿ��ѭ��ʱҪ������� index���� m_lkNodesTouch �� w ��С��
	long distance=0;			// ��ʱ����
	long i=0, j=0, t=0, ix=0;	// ��ʱ����
	CBArrLink * pe = NULL;		// ��ʱ����
	bool blRetCallBack=false;	// �ص���������ֵ

	if (m_NodesCount<=0 || m_Nodes==NULL)
	{
		_tcscpy(ErrDescription, TEXT("Calculating path failed: No data nodes exist. ") );	// "����·��ʧ�ܣ�û���κν�����ݡ�" 
		if (ShowMsgIfFail) MsgBox(ErrDescription, TEXT("Dijkstra Calculating path failed"), mb_OK, mb_IconExclamation);
		return false;
	}


	// �ӽ�� id �����ʼ��� index => iStart
	iStart=m_hashNodesIdxes.Item(idNodeStart,false);
	if (iStart<=0) 
	{
		_tcscpy(ErrDescription, StrAppend(TEXT("Calculating path failed: not found Starting NODE '"), Str(idNodeStart), TEXT("'. ") ) );		// "����·��ʧ�ܣ�û���ҵ���ʼ��� "
		if (ShowMsgIfFail) MsgBox(ErrDescription, TEXT("Dijkstra Calculating path failed"), mb_OK, mb_IconExclamation);
		return false;
	}


	// ============ ׼������ ============
	// �����н��� w ��Ϊ�����fVisited��Ϊ false��������н��Ļ��ݸ����
	for (i=1; i<=m_NodesCount; i++)
	{
		m_Nodes[i].w = mcMaxDistance;
		m_Nodes[i].fVisited = false;

		if (m_Nodes[i].pParentList) 
			m_Nodes[i].pParentList->Clear();
		else	// �ݴ�Ӧ�������� LoadFileData ʱ = new ��
			m_Nodes[i].pParentList = new CBArrLink;
	}
	m_indexStart = 0;					// ��ǰ m_Nodes[] ������¼�����·��״̬����ʼ�����Ч
	m_lkNodesTouch.Clear();				// ��������漰��㡱Ϊ��
	memset(ErrDescription, 0, sizeof(ErrDescription));	// ���������Ϣ

	// ������ʼ���� fVisited
	m_Nodes[iStart].fVisited = true;	// ������ʼ��� visited=true ����ʼ��㲻�� w��pParentList��
	ctVisits = 1;						// ����1����� visited
	
	// ��������ʼ���ֱ�����������н�㣬����Щ��㣺
	//   �������� w Ϊ�������ߵĳ��ȡ���������ݸ���㣨pParentList��Ϊ��ʼ���
	//   ���ҽ������ m_lkNodesTouch Ϊ��ʼ���漰�����Ľ��
	pe = m_Nodes[iStart].pEdges;
	t = pe->Count();
	for (i=1; i<=t; i++)
	{
		// �������ʼ���ֱ��������һ����� �� index��distance => index��distance
		ix = pe->Item(i);  distance = pe->Item2(i);
		// ��������ʼ���ֱ����������һ������ w Ϊ distance
		m_Nodes[ix].w = distance;  
		// ��������ʼ���ֱ����������һ�����Ļ��ݸ����Ϊ����ʼ���
		m_Nodes[ix].pParentList->Clear();
		m_Nodes[ix].pParentList->Add(iStart);

		// �� ����ʼ���ֱ����������һ����� ���� m_lkNodesTouch
		m_lkNodesTouch.Add (ix);
	}
	
	// ============ ѭ�� m_lkNodesTouch �е����н�㣨��ѭ���� m_lkNodesTouch ���ᱻ���������ݣ� ============
	int ctCycled=0;

	// ����һ�λص�����
	blRetCallBack=(*pf)(ctCycled, m_lkNodesTouch.Count(), userDataCallBack);
	if (! blRetCallBack) 
	{
		_tcscpy(ErrDescription, 
			_TEXT("\nFinding path not finished. Break by the user. ")); 
		return 0;	// break the calculation
	}

	// ÿ�δ� m_lkNodesTouch ��ȡһ�� w ��С�Ľ����в���������˽���� visited
	while (m_lkNodesTouch.Count()>0 && ctVisits<m_NodesCount-1)
	{	// ��� m_lkNodesTouch.Count()==0 Ϊ�գ���ʹ ctVisits<m_NodesCount-1 Ҳ������
		//	���� �� ��û��Ҫ�� �� ��ѡһ������ �� ������ȥ�ۼӾ���

		// ==== ȡ m_lkNodesTouch �� w ��С�Ľ�� => iVertex, ���� m_lkNodesTouch �е��±� =>j ====
		iVertex=-1;
		j=0; 
		distance=mcMaxDistance;
		t=m_lkNodesTouch.Count();
		for (i=1; i<=t; i++)
		{
			ix = m_lkNodesTouch.Item(i);
			if ( m_Nodes[ix].w < distance ) { j=i; distance=m_Nodes[ix].w; }
		}
		iVertex = m_lkNodesTouch.Item(j);
		if (iVertex<=0)	// �ݴ�
		{
			_tcscpy(ErrDescription, StrAppend(TEXT("Calculating path failed: Can not find a NODE with minimum w in the "), Str(ctVisits+1), TEXT("th cycle. ") ) );     // "����·��ʧ�ܣ��� "   " ��ѭ�������ҵ� w ֵ��С�Ľ�㡣"
			if (ShowMsgIfFail) MsgBox(ErrDescription, TEXT("Dijkstra Calculating path failed"), mb_OK, mb_IconExclamation);
			return false;
		}

		// ==== ���ñ�־ ====
		m_Nodes[iVertex].fVisited = true;	// ���ô˽���� visited
		ctVisits++;							// �� visited ����� +1
		m_lkNodesTouch.Remove(j);			// �� m_lkNodesTouch ��ɾ����һ���

		// ==== �ڲ�ѭ����ѭ�������� iVertex ֱ����������δ visited �Ľ�� ====
		// �� ix ����ÿ��ѭ��Ҫ������������һ������ index
		pe = m_Nodes[iVertex].pEdges; 
		t = pe->Count();
		for (i=1; i<=t; i++)
		{
			ix = pe->Item(i);			// �� ix ����ÿ��ѭ��Ҫ������������һ������ index
			distance = pe->Item2(i);	// �� iVertex �� ix �ı�ȨֵΪ pe->Item2(i)

			if ( m_Nodes[ix].fVisited ) continue; 

			// ���� m_Nodes[ix].w
			if ( m_Nodes[ix].w >=mcMaxDistance )
			{
				// m_Nodes[ix].w ��������󱻸��µ�
				m_Nodes[ix].w = m_Nodes[iVertex].w + distance;	// ���� m_Nodes[ix].w
				m_Nodes[ix].pParentList->Clear();				// ���� m_Nodes[ix] �Ļ��ݸ����Ϊ iVertex
				m_Nodes[ix].pParentList->Add(iVertex);

				// ��� m_Nodes[ix].w ��������󱻸��µģ����� m_lkNodesTouch ����� ix
				m_lkNodesTouch.Add(ix);
			}
			else	// if ( m_Nodes[ix].w >=mcMaxValue )
			{
				// ����ֻ����� m_Nodes[ix].w ����
				if ( m_Nodes[iVertex].w + distance < m_Nodes[ix].w ) 
				{
					m_Nodes[ix].w = m_Nodes[iVertex].w + distance;	// ���� m_Nodes[ix].w
					m_Nodes[ix].pParentList->Clear();				// ���� m_Nodes[ix] �Ļ��ݸ����Ϊ iVertex
					m_Nodes[ix].pParentList->Add(iVertex);
				}
				else if ( m_Nodes[iVertex].w + distance == m_Nodes[ix].w ) 
				{
					// �������·��
					m_Nodes[ix].pParentList->Add(iVertex);			// �� m_Nodes[ix] �Ļ��ݸ���������һ����� iVertex
				}	// if ( m_Nodes[iVertex].w + distance < m_Nodes[ix].w ) 
			}	// if ( m_Nodes[ix].w >=mcMaxValue ) // �Ƿ� m_Nodes[ix].w �������
		}	// for (i=1; i<=t; i++)
		// ==== �ڲ�ѭ������ ====
		// ====================================================================

		ctCycled++;
		if (pf && ctCycled%299==0)
		{
			blRetCallBack=(*pf)(ctCycled, m_lkNodesTouch.Count(), userDataCallBack);
			if (! blRetCallBack) 
			{ 
				_tcscpy(ErrDescription, 
					_TEXT("\nFinding path not finished. Break by the user. ")); 
				return 0;	// break the calculation
			}		
		}
	}	// end of while

	// ��ɣ�����һ�λص�����
	blRetCallBack=(*pf)(ctCycled, 0, userDataCallBack);	// ��2��������0����ʾ���
	if (! blRetCallBack) 
	{
		_tcscpy(ErrDescription, 
			_TEXT("\nFinding path not finished. Break by the user. ")); 
		return 0;	// break the calculation
	}

	// �ͷ� pe
	pe = NULL;

	// ���õ�ǰ m_Nodes[] ������¼�����·��״̬����ʼ���Ϊ��Чֵ
	m_indexStart = iStart;
	
	// ============ ����ֵ ============
	return true;
}






int CBDijkstra::NodeCount()
{
	return m_NodesCount;	
}

int CBDijkstra::NodeIndex( long idNode )
{
	return m_hashNodesIdxes.Item(idNode, false);
}


long CBDijkstra::NodeID( int idxNode )
{
	if (idxNode<1 || idxNode>m_NodesCount)
		return 0;	// for error
	else
		return m_Nodes[idxNode].ID;
}

int CBDijkstra::NodeAdjEdgesCount( int idxNode )
{
	if (idxNode<1 || idxNode>m_NodesCount)
		return 0;	// for error
	else
	{
		if (m_Nodes[idxNode].pEdges==NULL)
			return 0;	// for error
		else
			return m_Nodes[idxNode].pEdges->Count();	
	}
}

long CBDijkstra::NodeOneAdjEdge( int idxNode, int idxEdge, int &idxNodeAdj )
{
	idxNodeAdj = 0;  // set firstly. If error occurred below, idxNodeAdj is still be set to 0

	if (idxNode<1 || idxNode>m_NodesCount)
		return 0;	// for error
	else
	{
		if (m_Nodes[idxNode].pEdges==NULL)
			return 0;	// for error
		else
		{
			if (idxEdge<1 || idxNodeAdj>m_Nodes[idxNode].pEdges->Count())
				return 0;	// for error
			else
			{
				idxNodeAdj = m_Nodes[idxNode].pEdges->Item(idxEdge);
				return m_Nodes[idxNode].pEdges->Item2(idxEdge);
			}
		}
	}	
}

bool CBDijkstra::AddNodesDist( long idNode1, long idNode2, long distance )
{
	long idNode[3]={0}; int indexNode[3]={0};  // �������� ID��Index
						//���±��1��ʼ��0�˷Ѳ��ã�����������3���ռ䣩
	int iFlds=0, i=0, k=0; 

	// �ҵ���� idNode[i] �� m_Nodes[] �����е��±� => idxNode[i]
	//   �����޴˽�㣬����Ӵ˽�㣬Ȼ�������±� => idxNode[i]
	idNode[1] = idNode1; idNode[2] = idNode2;
	for (i=1; i<=2; i++)	// ���� 2 �����
	{
		if (! m_hashNodesIdxes.IsKeyExist(idNode[i]) )	// ����δ��ӹ��˽��
		{
			// ========================================
			// ����½�� idNode[i] 
			m_NodesCount ++;

			// ׼���ռ�
			if (m_NodesCount>m_NodesUbound)
			{
				// ���� m_Nodes �Ŀռ�
				Redim(m_Nodes, m_NodesUbound+mcNodesExpandStep, m_NodesUbound, true);
				m_NodesUbound = m_NodesUbound+mcNodesExpandStep; 
			}

			// �� m_Nodes[m_NodesCount] ������½�� idNode[i]
			m_Nodes[m_NodesCount].fVisited = false;
			m_Nodes[m_NodesCount].ID = idNode[i];
			m_Nodes[m_NodesCount].pEdges = new CBArrLink;
			m_Nodes[m_NodesCount].w = mcMaxDistance;		// �����ֵ
			m_Nodes[m_NodesCount].pParentList = new CBArrLink;	
			m_Nodes[m_NodesCount].tag = NULL;

			// ���½���±� �� idNode[i] �Ķ�Ӧ��ϵ����¼����ϣ��
			if (! m_hashNodesIdxes.Add(m_NodesCount, idNode[i], 0, 0, 0, 0, 0.0, false))
			{
				_tcscpy(ErrDescription, StrAppend( 
					TEXT("Can not add NODE "), Str(idNode[i]), TEXT(". ")));    // "������ӽ�� "
				return false;
			}
			// ����½�� idNode[i]���
			// ========================================

			// �½�� idNode[i] �� index => indexNode[i]
			indexNode[i] = m_NodesCount;
		} 
		else // if (! m_hashNodesIdxes.IsKeyExist(idNode[i]) )	// ����δ��ӹ��˽��
		{
			// �Ѿ���ӹ��˽�㣬���ظ���ӣ����� idNode[i] �ҵ��� index => indexNode[i]
			indexNode[i] = m_hashNodesIdxes.Item(idNode[i], false);
		}	// end if (! m_hashNodesIdxes.IsKeyExist(idNode[i]) )	// ����δ��ӹ��˽��
	}	// end for (i=1; i<=2; i++) // ���� 2 �����

	CBArrLink *phs; int idxNode;
	for(i=1; i<=2; i++)		// ���� 2 �����
	{
		// �� indexNode[1]������ indexNode[2] ���������� distance ��Ϣ��ӵ� m_Nodes[indexNode[1]]
		// �� indexNode[2]������ indexNode[1] ���������� distance ��Ϣ��ӵ� m_Nodes[indexNode[2]]
		phs = m_Nodes[indexNode[i]].pEdges;
		if (i==1) idxNode = indexNode[2]; else idxNode = indexNode[1];

		// ����� indexNode[i] �ġ��ߡ��������Ƿ��Ѽ�¼�������� idxNode �����ľ���
		for (k = 1; k <= phs->Count(); k++)
			if ( phs->Item(k) == idxNode ) break;
		if ( k <= phs->Count() )
		{
			//   �Ѽ�¼�˸þ��롣У���¼�ľ����Ƿ�� iDis һ�£���һ�����£��粻һ�£�������
			if (phs->Item2(k) != distance)
			{
				_tcscpy(ErrDescription, StrAppend( 
					TEXT("Found confilict duplicate distances between NODE '"), Str(m_Nodes[indexNode[i]].ID), 
					TEXT("' and NODE '"), Str(m_Nodes[idxNode].ID), TEXT("': "), Str(distance), 
					TEXT(". Another distance already set is "), Str(phs->Item2(k)), TEXT(".") ));    
				return false;
			}
			// ���˴�˵����Ȼ���ظ���¼�����Ѽ�¼�ĺͱ��ε� iDis һ�£���������ظ����ߡ����룬ֱ����һ������
		}	// if (k<=phs->Count())
		else
		{
			// ����µġ��ߡ�����
			phs->Add(idxNode, distance);	// node index, distance

		}	// end if (k<=phs->Count()) -- else
		phs=NULL;
	}

	return true;
}




// void CBDijkstra::PrintLinks()
// {
// 	int i,j;
// 	for (i=1; i<=m_NodesCount; i++)
// 	{
// 		cout<<i<<":"<<m_Nodes[i].ID<<"\t";
// 		for (j=1; j<=m_Nodes[i].pEdges->Count(); j++)
// 			cout<<m_Nodes[i].pEdges->Item(j)<<"("<<m_Nodes[i].pEdges->Item2(j)<<") ";
// 		cout<<endl;
// 	}
// 	cout<<endl<<endl<<"��ϣ�������"<<endl;
// 	cout<<m_hashNodesIdxes.Count()<<endl;
// 	for (i=1; i<=m_hashNodesIdxes.Count(); i++)
// 		cout<<m_hashNodesIdxes.IndexToKey(i)<<" "<<m_hashNodesIdxes.Item(i)<<endl;
// }

