#include "PW_Obj.h"
#include <vector>

using namespace std;

void PW_Mesh::ComputeNormal()
{
	if (indexcount == 0)
	{
		return;
	}
	vector<vector<PW_Vector3D>> vNormals;
	vNormals.resize(pointcount);
	for (int i = 0; i < indexcount;i+=3)
	{
		int index1 = indexbuffer[i];
		int index2 = indexbuffer[i + 1];
		int index3 = indexbuffer[i + 2];
		PW_Vector3D v = buffer[index2] - buffer[index1];
		PW_Vector3D q = buffer[index3] - buffer[index2];
		PW_Vector3D nor;
		PW_CrossProduct(v, q, nor);
		nor.Normalize();
		vNormals[index1].push_back(nor);
		vNormals[index2].push_back(nor);
		vNormals[index3].push_back(nor);
	}
	for (int i = 0; i < pointcount;i++)
	{
		PW_Vector3D nor;
		for (int k = 0; k < vNormals[i].size(); ++k)
		{
			nor = nor + vNormals[i][k];
			//nor.Normalize();
		}
		nor.Normalize();
		buffer[i].vNormal = nor;
	}
}