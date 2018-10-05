#include "stdafx.h"
#include "IrradianceMapGen.h"

namespace {
	using namespace DirectX::SimpleMath;
	//https://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering/
	//SEBASTIEN LAGARDE'S WORK to get texel to vector, vector to texel and solid angle

	//used to index cube faces
	#define CP_FACE_X_POS 0
	#define CP_FACE_X_NEG 1
	#define CP_FACE_Y_POS 2
	#define CP_FACE_Y_NEG 3
	#define CP_FACE_Z_POS 4
	#define CP_FACE_Z_NEG 5

	//used to index image edges
	// NOTE.. the actual number corresponding to the edge is important
	//  do not change these, or the code will break
	//
	// CP_EDGE_LEFT   is u = 0
	// CP_EDGE_RIGHT  is u = width-1
	// CP_EDGE_TOP    is v = 0
	// CP_EDGE_BOTTOM is v = height-1
	#define CP_EDGE_LEFT   0
	#define CP_EDGE_RIGHT  1
	#define CP_EDGE_TOP    2
	#define CP_EDGE_BOTTOM 3

	//corners of CUBE map (P or N specifys if it corresponds to the 
	//  positive or negative direction each of X, Y, and Z
	#define CP_CORNER_NNN  0
	#define CP_CORNER_NNP  1
	#define CP_CORNER_NPN  2
	#define CP_CORNER_NPP  3
	#define CP_CORNER_PNN  4
	#define CP_CORNER_PNP  5
	#define CP_CORNER_PPN  6
	#define CP_CORNER_PPP  7

	//information about cube maps neighboring face after traversing
	// across an edge
	struct CPCubeMapNeighbor
	{
		uint8_t m_Face;    //index of neighboring face
		uint8_t m_Edge;    //edge in neighboring face that abuts this face
	};

	//------------------------------------------------------------------------------
	// D3D cube map face specification
	//   mapping from 3D x,y,z cube map lookup coordinates 
	//   to 2D within face u,v coordinates
	//
	//   --------------------> U direction 
	//   |                   (within-face texture space)
	//   |         _____
	//   |        |     |
	//   |        | +Y  |
	//   |   _____|_____|_____ _____
	//   |  |     |     |     |     |
	//   |  | -X  | +Z  | +X  | -Z  |
	//   |  |_____|_____|_____|_____|
	//   |        |     |
	//   |        | -Y  |
	//   |        |_____|
	//   |
	//   v   V direction
	//      (within-face texture space)
	//------------------------------------------------------------------------------

	//Information about neighbors and how texture coorrdinates change across faces 
	//  in ORDER of left, right, top, bottom (e.g. edges corresponding to u=0, 
	//  u=1, v=0, v=1 in the 2D coordinate system of the particular face.
	//Note this currently assumes the D3D cube face ordering and orientation
	CPCubeMapNeighbor sg_CubeNgh[6][4] =
	{
		//XPOS face
		{ { CP_FACE_Z_POS, CP_EDGE_RIGHT },
	{ CP_FACE_Z_NEG, CP_EDGE_LEFT },
	{ CP_FACE_Y_POS, CP_EDGE_RIGHT },
	{ CP_FACE_Y_NEG, CP_EDGE_RIGHT } },
	//XNEG face
	{ { CP_FACE_Z_NEG, CP_EDGE_RIGHT },
	{ CP_FACE_Z_POS, CP_EDGE_LEFT },
	{ CP_FACE_Y_POS, CP_EDGE_LEFT },
	{ CP_FACE_Y_NEG, CP_EDGE_LEFT } },
	//YPOS face
	{ { CP_FACE_X_NEG, CP_EDGE_TOP },
	{ CP_FACE_X_POS, CP_EDGE_TOP },
	{ CP_FACE_Z_NEG, CP_EDGE_TOP },
	{ CP_FACE_Z_POS, CP_EDGE_TOP } },
	//YNEG face
	{ { CP_FACE_X_NEG, CP_EDGE_BOTTOM },
	{ CP_FACE_X_POS, CP_EDGE_BOTTOM },
	{ CP_FACE_Z_POS, CP_EDGE_BOTTOM },
	{ CP_FACE_Z_NEG, CP_EDGE_BOTTOM } },
	//ZPOS face
	{ { CP_FACE_X_NEG, CP_EDGE_RIGHT },
	{ CP_FACE_X_POS, CP_EDGE_LEFT },
	{ CP_FACE_Y_POS, CP_EDGE_BOTTOM },
	{ CP_FACE_Y_NEG, CP_EDGE_TOP } },
	//ZNEG face
	{ { CP_FACE_X_POS, CP_EDGE_RIGHT },
	{ CP_FACE_X_NEG, CP_EDGE_LEFT },
	{ CP_FACE_Y_POS, CP_EDGE_TOP },
	{ CP_FACE_Y_NEG, CP_EDGE_BOTTOM } }
	};


	//3x2 matrices that map cube map indexing vectors in 3d 
	// (after face selection and divide through by the 
	//  _ABSOLUTE VALUE_ of the max coord)
	// into NVC space
	//Note this currently assumes the D3D cube face ordering and orientation
#define CP_UDIR     0
#define CP_VDIR     1
#define CP_FACEAXIS 2

	float sgFace2DMapping[6][3][3] = {
		//XPOS face
		{ { 0,  0, -1 },   //u towards negative Z
	{ 0, -1,  0 },   //v towards negative Y
	{ 1,  0,  0 } },  //pos X axis  
					  //XNEG face
	{ { 0,  0,  1 },   //u towards positive Z
	{ 0, -1,  0 },   //v towards negative Y
	{ -1,  0,  0 } },  //neg X axis       
					   //YPOS face
	{ { 1, 0, 0 },     //u towards positive X
	{ 0, 0, 1 },     //v towards positive Z
	{ 0, 1 , 0 } },   //pos Y axis  
					  //YNEG face
	{ { 1, 0, 0 },     //u towards positive X
	{ 0, 0 , -1 },   //v towards negative Z
	{ 0, -1 , 0 } },  //neg Y axis  
					  //ZPOS face
	{ { 1, 0, 0 },     //u towards positive X
	{ 0, -1, 0 },    //v towards negative Y
	{ 0, 0,  1 } },   //pos Z axis  
					  //ZNEG face
	{ { -1, 0, 0 },    //u towards negative X
	{ 0, -1, 0 },    //v towards negative Y
	{ 0, 0, -1 } },   //neg Z axis  
	};


	//The 12 edges of the cubemap, (entries are used to index into the neighbor table)
	// this table is used to average over the edges.
	int sg_CubeEdgeList[12][2] = {
		{ CP_FACE_X_POS, CP_EDGE_LEFT },
	{ CP_FACE_X_POS, CP_EDGE_RIGHT },
	{ CP_FACE_X_POS, CP_EDGE_TOP },
	{ CP_FACE_X_POS, CP_EDGE_BOTTOM },

	{ CP_FACE_X_NEG, CP_EDGE_LEFT },
	{ CP_FACE_X_NEG, CP_EDGE_RIGHT },
	{ CP_FACE_X_NEG, CP_EDGE_TOP },
	{ CP_FACE_X_NEG, CP_EDGE_BOTTOM },

	{ CP_FACE_Z_POS, CP_EDGE_TOP },
	{ CP_FACE_Z_POS, CP_EDGE_BOTTOM },
	{ CP_FACE_Z_NEG, CP_EDGE_TOP },
	{ CP_FACE_Z_NEG, CP_EDGE_BOTTOM }
	};


	//Information about which of the 8 cube corners are correspond to the 
	//  the 4 corners in each cube face
	//  the order is upper left, upper right, lower left, lower right
	int sg_CubeCornerList[6][4] = {
		{ CP_CORNER_PPP, CP_CORNER_PPN, CP_CORNER_PNP, CP_CORNER_PNN }, // XPOS face
	{ CP_CORNER_NPN, CP_CORNER_NPP, CP_CORNER_NNN, CP_CORNER_NNP }, // XNEG face
	{ CP_CORNER_NPN, CP_CORNER_PPN, CP_CORNER_NPP, CP_CORNER_PPP }, // YPOS face
	{ CP_CORNER_NNP, CP_CORNER_PNP, CP_CORNER_NNN, CP_CORNER_PNN }, // YNEG face
	{ CP_CORNER_NPP, CP_CORNER_PPP, CP_CORNER_NNP, CP_CORNER_PNP }, // ZPOS face
	{ CP_CORNER_PPN, CP_CORNER_NPN, CP_CORNER_PNN, CP_CORNER_NNN }  // ZNEG face
	};

	void VectToTexelCoord(const Vector3& normalizedDir, const uint32_t faceDimension, int& faceID, int& u, int& v)
	{
		float nvcU, nvcV;
		Vector3 absDir;
		float maxCoord;
		Vector3 onFaceXYZ;

		//absolute value 3
		normalizedDir.Abs(absDir);

		if ((absDir.x >= absDir.y) && (absDir.x >= absDir.z))
		{
			maxCoord = absDir.x;

			if (normalizedDir.x >= 0) //face = XPOS
			{
				faceID = CP_FACE_X_POS;
			}
			else
			{
				faceID = CP_FACE_X_NEG;
			}
		}
		else if ((absDir.y >= absDir.x) && (absDir.y >= absDir.z))
		{
			maxCoord = absDir.y;

			if (absDir.y >= 0) //face = XPOS
			{
				faceID = CP_FACE_Y_POS;
			}
			else
			{
				faceID = CP_FACE_Y_NEG;
			}
		}
		else  // if( (absXYZ[2] > absXYZ[0]) && (absXYZ[2] > absXYZ[1]) )
		{
			maxCoord = absDir.z;

			if (absDir.z >= 0) //face = XPOS
			{
				faceID = CP_FACE_Z_POS;
			}
			else
			{
				faceID = CP_FACE_Z_NEG;
			}
		}

		//divide through by max coord so face vector lies on cube face
		onFaceXYZ = normalizedDir * (1.0f / maxCoord);

		Vector3 uDir = { sgFace2DMapping[faceID][CP_UDIR][0], sgFace2DMapping[faceID][CP_UDIR][1], sgFace2DMapping[faceID][CP_UDIR][2] };
		Vector3 vDir = { sgFace2DMapping[faceID][CP_VDIR][0], sgFace2DMapping[faceID][CP_VDIR][1], sgFace2DMapping[faceID][CP_VDIR][2] };
		nvcU = onFaceXYZ.Dot(uDir);
		nvcV = onFaceXYZ.Dot(vDir);

		//Return a value from 0 to Size - 1
		u = (int)floor((faceDimension - 1) * 0.5f * (nvcU + 1.0f));
		v = (int)floor((faceDimension - 1) * 0.5f * (nvcV + 1.0f));
	}

	//--------------------------------------------------------------------------------------
	// Convert cubemap face texel coordinates and face idx to 3D vector
	// note the U and V coords are integer coords and range from 0 to size-1
	//  this routine can be used to generate a normalizer cube map
	//--------------------------------------------------------------------------------------
	void TexelCoordToVect(const int& faceID, const float& u, const float& v, const uint32_t& faceDimension, Vector3& direction)
	{
		float nvcU, nvcV;
		Vector3 tempVec;

		// transform from [0..res - 1] to [- (1 - 1 / res) .. (1 - 1 / res)]
		// + 0.5f is for texel center addressing
		nvcU = (2.0f * ((float)u + 0.5f) / (float)faceDimension) - 1.0f;
		nvcV = (2.0f * ((float)v + 0.5f) / (float)faceDimension) - 1.0f;

		//generate x,y,z vector (xform 2d NVC coord to 3D vector)
		//U contribution
		direction = Vector3(sgFace2DMapping[faceID][CP_UDIR][0], sgFace2DMapping[faceID][CP_UDIR][1], sgFace2DMapping[faceID][CP_UDIR][2]) * nvcU;
		//V contribution
		tempVec = Vector3(sgFace2DMapping[faceID][CP_VDIR][0], sgFace2DMapping[faceID][CP_VDIR][1], sgFace2DMapping[faceID][CP_VDIR][2]) * nvcV;
		direction = direction + tempVec;

		//add face axis
		direction = direction + Vector3(sgFace2DMapping[faceID][CP_FACEAXIS][0], sgFace2DMapping[faceID][CP_FACEAXIS][1], sgFace2DMapping[faceID][CP_FACEAXIS][2]);

		//normalize vector
		direction.Normalize();
	}

	/** Original code from Ignacio Castaño
	* This formula is from Manne Öhrström's thesis.
	* Take two coordiantes in the range [-1, 1] that define a portion of a
	* cube face and return the area of the projection of that portion on the
	* surface of the sphere.
	**/
	float AreaElement(float x, float y)
	{
		return atan2(x * y, sqrt(x * x + y * y + 1));
	}

	//takes as input integer u,v in range [0, faceDimension-1]
	void TexelCoordSolidAngle(uint32_t u, uint32_t v, const uint32_t& faceDimension, float& solidAngle)
	{
		//scale up to [-1, 1] range (inclusive), offset by 0.5 to point to texel center.
		u = (2.0f * ((float)u + 0.5f) / (float)faceDimension) - 1.0f;
		v = (2.0f * ((float)v + 0.5f) / (float)faceDimension) - 1.0f;

		float InvResolution = 1.0f / faceDimension;

		// U and V are the -1..1 texture coordinate on the current face.
		// Get projected area for this texel
		float x0 = u - InvResolution;
		float y0 = v - InvResolution;
		float x1 = u + InvResolution;
		float y1 = v + InvResolution;
		solidAngle = AreaElement(x0, y0) - AreaElement(x0, y1) - AreaElement(x1, y0) + AreaElement(x1, y1);
	}

	//Jacks code here
	//Always 2nd order for now
	void ProjectOntoSH(const std::vector<uint8_t*>& cubeFacePtrs, const uint32_t& faceDimension, const uint32_t& numChannels, std::vector<Vector3>& projectionCoeffs)
	{
		Vector3 direction;
		float deltaSolidAngle;
		float totalSolidAngle = 0.0f;
		uint8_t* cubeFacePtr;
		uint32_t texelByteOffset;
		Vector3 sampledColor; //I believe the file we are sampling is sRGB, but we are building a weighted sum, should we not go to linear color space first?

		const float shCoeff0 = 0.282095f;
		const float shCoeff1 = 0.488603f;
		const float shCoeff2 = 1.092548f;
		const float shCoeff3 = 0.315392f;
		const float shCoeff4 = 0.546274f;

		Vector3 coeffs0 = Vector3::Zero;
		Vector3 coeffs1 = Vector3::Zero;
		Vector3 coeffs2 = Vector3::Zero;
		Vector3 coeffs3 = Vector3::Zero;
		Vector3 coeffs4 = Vector3::Zero;
		Vector3 coeffs5 = Vector3::Zero;
		Vector3 coeffs6 = Vector3::Zero;
		Vector3 coeffs7 = Vector3::Zero;
		Vector3 coeffs8 = Vector3::Zero;
		for (uint8_t faceID = 0; faceID < 6; faceID++) {
			for (uint32_t v = 0; v < faceDimension; v++) {
				for (uint32_t u = 0; u < faceDimension; u++) {
					TexelCoordToVect(faceID, u, v, faceDimension, direction);
					TexelCoordSolidAngle(u, v, faceDimension, deltaSolidAngle);
					totalSolidAngle += deltaSolidAngle;

					cubeFacePtr = cubeFacePtrs[faceID];
					texelByteOffset = (faceDimension * v + u) * numChannels;
					/*sampledColor = Vector3((float)*(cubeFacePtr + texelByteOffset), (float)*(cubeFacePtr + texelByteOffset + 1),
										(float)*(cubeFacePtr + texelByteOffset + 2));*/
					sampledColor = Vector3((float)*(cubeFacePtr + texelByteOffset + 2), (float)*(cubeFacePtr + texelByteOffset + 1),
						(float)*(cubeFacePtr + texelByteOffset));

					coeffs0 += shCoeff0 * deltaSolidAngle * sampledColor;
					coeffs1 += shCoeff1 * deltaSolidAngle * sampledColor * direction.x;
					coeffs2 += shCoeff1 * deltaSolidAngle * sampledColor * direction.z;
					coeffs3 += shCoeff1 * deltaSolidAngle * sampledColor * direction.y;
					coeffs4 += shCoeff2 * deltaSolidAngle * sampledColor * direction.x * direction.z;
					coeffs5 += shCoeff2 * deltaSolidAngle * sampledColor * direction.y * direction.z;
					coeffs6 += shCoeff2 * deltaSolidAngle * sampledColor * direction.x * direction.y;
					coeffs7 += shCoeff3 * deltaSolidAngle * sampledColor * (3 * direction.z * direction.z - 1);
					coeffs8 += shCoeff4 * deltaSolidAngle * sampledColor * ((direction.x * direction.x) - (direction.y * direction.y));
				}
			}
		}

		DebugLog("sf", "Total solid angle is: ", totalSolidAngle);

		//normalize the solid angle contribution, and also convert 0-255 color values to [0-1] range
		float solidAngleNormalization = 4.0 * DirectX::XM_PI / totalSolidAngle;
		coeffs0 *= solidAngleNormalization / 255.0f;
		coeffs1 *= solidAngleNormalization / 255.0f;
		coeffs2 *= solidAngleNormalization / 255.0f;
		coeffs3 *= solidAngleNormalization / 255.0f;
		coeffs4 *= solidAngleNormalization / 255.0f;
		coeffs5 *= solidAngleNormalization / 255.0f;
		coeffs6 *= solidAngleNormalization / 255.0f;
		coeffs7 *= solidAngleNormalization / 255.0f;
		coeffs8 *= solidAngleNormalization / 255.0f;

		projectionCoeffs.emplace_back(coeffs0);
		projectionCoeffs.emplace_back(coeffs1);
		projectionCoeffs.emplace_back(coeffs2);
		projectionCoeffs.emplace_back(coeffs3);
		projectionCoeffs.emplace_back(coeffs4);
		projectionCoeffs.emplace_back(coeffs5);
		projectionCoeffs.emplace_back(coeffs6);
		projectionCoeffs.emplace_back(coeffs7);
		projectionCoeffs.emplace_back(coeffs8);
	}

	//takes 9 Vector3s as the SH coefficients for radiance as RGB vectors, and scales them to irradiance
	void RadianceToIrradianceSH(std::vector<Vector3>& coefficients) 
	{
		float c1 = 3.141593f * 0.282095f;
		float c2 = 2.094395f * 0.488603f;
		float c3 = 0.785398f * 1.092548f;
		float c4 = 0.785398f * 0.315392f;
		float c5 = 0.785398f * 0.546274f;

		coefficients[0] *= c1;
		coefficients[1] *= c2;
		coefficients[2] *= c2;
		coefficients[3] *= c2;
		coefficients[4] *= c3;
		coefficients[5] *= c3;
		coefficients[6] *= c3;
		coefficients[7] *= c4;
		coefficients[8] *= c5;
	}
}

void IrradianceMapGen::GenerateIrradianceSH(const std::vector<uint8_t*>& cubeFacePtrs, const uint32_t& faceDimension, const uint32_t& numChannels)
{
	assert(numChannels == 4);

	//std::vector<Vector3> irradianceSH;
	//ProjectOntoSH(cubeFacePtrs, faceDimension, numChannels, irradianceSH);
	//RadianceToIrradianceSH(irradianceSH);
	//DebugLog("sfsfsf", "Coefficient 0 is: ", irradianceSH[0].x, ", ", irradianceSH[0].y, ", ", irradianceSH[0].z);
	//DebugLog("sfsfsf", "Coefficient 1 is: ", irradianceSH[1].x, ", ", irradianceSH[1].y, ", ", irradianceSH[1].z);
	//DebugLog("sfsfsf", "Coefficient 2 is: ", irradianceSH[2].x, ", ", irradianceSH[2].y, ", ", irradianceSH[2].z);
	//DebugLog("sfsfsf", "Coefficient 3 is: ", irradianceSH[3].x, ", ", irradianceSH[3].y, ", ", irradianceSH[3].z);
	//DebugLog("sfsfsf", "Coefficient 4 is: ", irradianceSH[4].x, ", ", irradianceSH[4].y, ", ", irradianceSH[4].z);
	//DebugLog("sfsfsf", "Coefficient 5 is: ", irradianceSH[5].x, ", ", irradianceSH[5].y, ", ", irradianceSH[5].z);
	//DebugLog("sfsfsf", "Coefficient 6 is: ", irradianceSH[6].x, ", ", irradianceSH[6].y, ", ", irradianceSH[6].z);
	//DebugLog("sfsfsf", "Coefficient 7 is: ", irradianceSH[7].x, ", ", irradianceSH[7].y, ", ", irradianceSH[7].z);
	//DebugLog("sfsfsf", "Coefficient 8 is: ", irradianceSH[8].x, ", ", irradianceSH[8].y, ", ", irradianceSH[8].z);
}