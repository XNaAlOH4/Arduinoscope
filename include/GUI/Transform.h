#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <CustoMath.h>

#define Ortho_matP(vals) Ortho_mat(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5])
matrix Ortho_mat(float left, float right, float bottom, float top, float fNear, float fFar)
{
	return init_build_matrix(4, 4, 7,
			0, 2/(right-left),
			5, 2/(top-bottom),
			10, -2/(fFar - fNear),
			12, -(right + left) / (right - left),
			13, -(top + bottom) / (top - bottom),
			14, -(fFar + fNear) / (fFar - fNear),
			15, 1.f);
}

#define Persp_matP(vals) Persp_mat(vals[0], vals[1], vals[2], vals[3])
matrix Persp_mat(float fFar, float fNear, float fFov, float fAspectRatio)
{
	float fFovRad = 1.f / (float)tan((double)(fFov/ 180.f * M_PI_2));
	return init_build_matrix(4, 4, 5,
			0, fAspectRatio * fFovRad,
			5, fFovRad,
			10, fFar/(fFar - fNear),
			11, fFar * fNear/(fNear - fFar),
			14, 1.f);
}

#define Persp_vecP(vals) Persp_vec(vals[0], vals[1], vals[2], vals[3])
vec Persp_vec(float fFar, float fNear, float fFov, float fAspectRatio)
{
	float fFovRad = 1.f / (float)tan((double)(fFov/180.f * M_PI_2));
	return build_vec(4, fAspectRatio * fFovRad,
			fFovRad,
			fFar/(fFar - fNear),
			fFar * fNear/(fNear - fFar));
}

#define Translate_matP(vals) Translate_mat(vals[0], vals[1], vals[2], vals[3])
matrix Translate_mat(float x, float y, float z, float scale)
{
	return init_build_matrix(4, 4, 7,
			0, scale,
			3, -x,
			5, scale,
			7, -y,
			10, scale,
			11, -z,
			15, 1.f);
}

#define Rot_matX(d) Rot_mat(d, 1)
#define Rot_matY(d) Rot_mat(d, 2)
#define Rot_matZ(d) Rot_mat(d, 4)

// x, y 0
// x, z 1
//
// y, x == x, -z 2
// y, z == x, y 0
//
// z, x 0
// z, y 1

// TODO double check all compound cases
matrix Rot_mat(double d, char axis)
{
	switch(axis) {
		case 1:// x
			return init_build_matrix(4, 4, 6,
					0, 1.f,
					5, cos(d),
					6, -sin(d),
					9, sin(d),
					10, cos(d),
					15, 1.f);
		case 2:// y
			return init_build_matrix(4, 4, 6,
					0, cos(d),
					2, sin(d),
					5, 1.f,
					8, -sin(d),
					10, cos(d),
					15, 1.f);
		case 4:// z
			return init_build_matrix(4, 4, 6,
					0, cos(d),
					1, -sin(d),
					4, sin(d),
					5, cos(d),
					10, 1.f,
					15, 1.f);
/*		case 1 | 2:// x, y
			double vals[] = {sin(d[0]), cos(d[0]), sin(d[1]), cos(d[1])};
			return init_build_matrix(9, 4, 4,
					0, vals[3],
					2, vals[2],
					4, vals[0] * vals[2],
					5, vals[1],
					6, -vals[0] * vals[3],
					8, -vals[1] * vals[2],
					9, vals[1],
					10, vals[1] * vals[3],
					15, 1.f);
		case 1 | 4:// x, z
			double vals[] = {sin(d[0]), cos(d[0]), sin(d[1]), cos(d[1])};
			return init_build_matrix(9, 4, 4,
					0, vals[3],
					2, -vals[2],
					4, vals[1] * vals[2],
					5, vals[1] * vals[3] - vals[0],
					6, -vals[0] * vals[3],
					8, -vals[1] * vals[2],
					9, vals[1],
					10, vals[1] * vals[3],
					15, 1.f);
		case 1 | 2 | 4:// x, y, z
			return;
		case 1 | 16 | 4:// x, z, y
			return;
		case 2 | 8:// y, x
			return;
		case 2 | 4:// y, z
			return;
		case 2 | 8 | 32:// y, x, z
			return;
		case 2 | 8 | 4:// y, z, x
			return;
		case 4 | 8:// z, x
			return;
		case 4 | 16:// z, y
			return;
		case 4 | 8 | 16:// z, x, y
			return;
		case 4 | 64 | 16:// z, y, x
			return;*/
		default:
			fprintf(stderr, "Error in rotMat, OPCODE[%d] not found\n", axis);
			exit(1);
			break;
	}
}

#endif
