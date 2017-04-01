//***************************************************************************//
//* File Name: quaternion.cpp                                               *//
//* Author:    Tom Portegys, portegys@ilstu.edu                             *//
//* Date Made: 07/25/02                                                     *//
//* File Desc: Implementation details representing a quaternion.            *//
//* Rev. Date:                                                              *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#include "Quaternion.h"

// Constructors.
CQuaternion::CQuaternion()
{
   m_quat[0]     = 0.0;
   m_quat[1]     = 0.0;
   m_quat[2]     = 0.0;
   m_quat[3]     = 1.0;
   m_normal_count = 0;
}


CQuaternion::CQuaternion(double quat[4])
{
   Init(quat);
   m_normal_count = 0;
}


// Initialize.
void CQuaternion::Init(double quat[4])
{
   m_quat[0] = quat[0];
   m_quat[1] = quat[1];
   m_quat[2] = quat[2];
   m_quat[3] = quat[3];
}


// Clear.
void CQuaternion::Clear()
{
   m_quat[0]     = 0.0;
   m_quat[1]     = 0.0;
   m_quat[2]     = 0.0;
   m_quat[3]     = 1.0;
   m_normal_count = 0;
}


void CQuaternion::VecZero(double *v)
{
   v[0] = 0.0;
   v[1] = 0.0;
   v[2] = 0.0;
}


void CQuaternion::VecSet(double *v, double x, double y, double z)
{
   v[0] = x;
   v[1] = y;
   v[2] = z;
}


void CQuaternion::VecSub(const double *src1, const double *src2, double *dst)
{
   dst[0] = src1[0] - src2[0];
   dst[1] = src1[1] - src2[1];
   dst[2] = src1[2] - src2[2];
}


void CQuaternion::VecCopy(const double *v1, double *v2)
{
   int i;

   for (int i = 0; i < 3; i++)
   {
      v2[i] = v1[i];
   }
}


void CQuaternion::VecCross(const double *v1, const double *v2, double *cross)
{
   double temp[3];

   temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
   temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
   temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);

   VecCopy(temp, cross);
}


double CQuaternion::VecLength(const double *v)
{
   return ((double)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]));
}


void CQuaternion::VecScale(double *v, double div)
{
   v[0] *= div;
   v[1] *= div;
   v[2] *= div;
}


void CQuaternion::VecNormal(double *v)
{
   VecScale(v, 1.0f / VecLength(v));
}


double CQuaternion::VecDot(const double *v1, const double *v2)
{
   return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}


void CQuaternion::VecAdd(const double *src1, const double *src2, double *dst)
{
   dst[0] = src1[0] + src2[0];
   dst[1] = src1[1] + src2[1];
   dst[2] = src1[2] + src2[2];
}


/*
 * Add quaternions, normalizing periodically.
 */

#define NORMALFREQ    100

void CQuaternion::AddQuats(CQuaternion& q1, CQuaternion& q2, CQuaternion& dest)
{
   double t1[4], t2[4], t3[4];
   double tf[4];

   VecCopy(q1.m_quat, t1);
   VecScale(t1, q2.m_quat[3]);
   VecCopy(q2.m_quat, t2);
   VecScale(t2, q1.m_quat[3]);
   VecCross(q2.m_quat, q1.m_quat, t3);
   VecAdd(t1, t2, tf);
   VecAdd(t3, tf, tf);
   tf[3]          = q1.m_quat[3] * q2.m_quat[3] - VecDot(q1.m_quat, q2.m_quat);
   dest.m_quat[0] = tf[0];
   dest.m_quat[1] = tf[1];
   dest.m_quat[2] = tf[2];
   dest.m_quat[3] = tf[3];

   if (++m_normal_count > NORMALFREQ)
   {
      m_normal_count = 0;
      dest.NormalizeQuat();
   }
}


/*
 * Multiply quaternions, normalizing periodically.
 */
void CQuaternion::MulQuats(CQuaternion& q1, CQuaternion& q2, CQuaternion& dest)
{
   dest.m_quat[3] = q2.m_quat[3] * q1.m_quat[3]
                    - q2.m_quat[0] * q1.m_quat[0]
                    - q2.m_quat[1] * q1.m_quat[1]
                    - q2.m_quat[2] * q1.m_quat[2];

   dest.m_quat[0] = q2.m_quat[3] * q1.m_quat[0]
                    + q2.m_quat[0] * q1.m_quat[3]
                    + q2.m_quat[1] * q1.m_quat[2]
                    - q2.m_quat[2] * q1.m_quat[1];

   dest.m_quat[1] = q2.m_quat[3] * q1.m_quat[1]
                    - q2.m_quat[0] * q1.m_quat[2]
                    + q2.m_quat[1] * q1.m_quat[3]
                    + q2.m_quat[2] * q1.m_quat[0];

   dest.m_quat[2] = q2.m_quat[3] * q1.m_quat[2]
                    + q2.m_quat[0] * q1.m_quat[1]
                    - q2.m_quat[1] * q1.m_quat[0]
                    + q2.m_quat[2] * q1.m_quat[3];

   if (++m_normal_count > NORMALFREQ)
   {
      m_normal_count = 0;
      dest.NormalizeQuat();
   }
}


/*
 * Quaternions always obey:  a^2 + b^2 + c^2 + d^2 = 1.0
 * If they don't add up to 1.0, dividing by their magnitude will
 * renormalize them.
 *
 * Note: See the following for more information on quaternions:
 *
 * - Shoemake, K., Animating rotation with quaternion curves, Computer
 *   Graphics 19, No 3 (Proc. SIGGRAPH'85), 245-254, 1985.
 * - Pletinckx, D., Quaternion calculus as a basic tool in computer
 *   graphics, The Visual Computer 5, 2-13, 1989.
 */
void CQuaternion::NormalizeQuat()
{
   int   i;
   double mag;

   mag = (m_quat[0] * m_quat[0] + m_quat[1] * m_quat[1] +
          m_quat[2] * m_quat[2] + m_quat[3] * m_quat[3]);
   for (i = 0; i < 4; i++)
   {
      m_quat[i] /= mag;
   }
}


/*
 * Build a rotation matrix, given a quaternion rotation.
 */
void CQuaternion::BuildRotationMatrix(double m[4][4])
{
   m[0][0] = 1.0f - 2.0f * (m_quat[1] * m_quat[1] + m_quat[2] * m_quat[2]);
   m[0][1] = 2.0f * (m_quat[0] * m_quat[1] - m_quat[2] * m_quat[3]);
   m[0][2] = 2.0f * (m_quat[2] * m_quat[0] + m_quat[1] * m_quat[3]);
   m[0][3] = 0.0;

   m[1][0] = 2.0f * (m_quat[0] * m_quat[1] + m_quat[2] * m_quat[3]);
   m[1][1] = 1.0f - 2.0f * (m_quat[2] * m_quat[2] + m_quat[0] * m_quat[0]);
   m[1][2] = 2.0f * (m_quat[1] * m_quat[2] - m_quat[0] * m_quat[3]);
   m[1][3] = 0.0;

   m[2][0] = 2.0f * (m_quat[2] * m_quat[0] - m_quat[1] * m_quat[3]);
   m[2][1] = 2.0f * (m_quat[1] * m_quat[2] + m_quat[0] * m_quat[3]);
   m[2][2] = 1.0f - 2.0f * (m_quat[1] * m_quat[1] + m_quat[0] * m_quat[0]);
   m[2][3] = 0.0;

   m[3][0] = 0.0;
   m[3][1] = 0.0;
   m[3][2] = 0.0;
   m[3][3] = 1.0;
}


// Load axis-angle rotation into quaternion.
void CQuaternion::LoadRotation(double angle, double *axis)
{
   VecNormal(axis);
   VecCopy(axis, m_quat);
   VecScale(m_quat, (double)sin(angle / 2.0f));
   m_quat[3] = (double)cos(angle / 2.0f);
   NormalizeQuat();
}


// Merge an axis-angle rotation into quaternion.
void CQuaternion::MergeRotation(double angle, double *axis)
{
   CQuaternion q1, q2;

   q2.m_quat[0] = axis[0] * (double)sin(angle / 2.0f);
   q2.m_quat[1] = axis[1] * (double)sin(angle / 2.0f);
   q2.m_quat[2] = axis[2] * (double)sin(angle / 2.0f);
   q2.m_quat[3] = (double)cos(angle / 2.0f);
   q2.NormalizeQuat();
   q1.m_quat[0] = m_quat[0];
   q1.m_quat[1] = m_quat[1];
   q1.m_quat[2] = m_quat[2];
   q1.m_quat[3] = m_quat[3];
   MulQuats(q2, q1, *this);
   NormalizeQuat();
}


// Make quaternion from Euler angles.
void CQuaternion::MakeQFromEulerAngles(double pitch, double yaw, double roll)
{
   // Re-orient internally.
   double iroll  = DegreesToRadians(pitch);
   double ipitch = DegreesToRadians(yaw);
   double iyaw   = DegreesToRadians(roll);

   double cyaw, cpitch, croll, syaw, spitch, sroll;
   double cyawcpitch, syawspitch, cyawspitch, syawcpitch;

   cyaw   = cos(0.5f * iyaw);
   cpitch = cos(0.5f * ipitch);
   croll  = cos(0.5f * iroll);
   syaw   = sin(0.5f * iyaw);
   spitch = sin(0.5f * ipitch);
   sroll  = sin(0.5f * iroll);

   cyawcpitch = cyaw * cpitch;
   syawspitch = syaw * spitch;
   cyawspitch = cyaw * spitch;
   syawcpitch = syaw * cpitch;

   m_quat[3] = (double)(cyawcpitch * croll + syawspitch * sroll);
   m_quat[0] = (double)(cyawcpitch * sroll - syawspitch * croll);
   m_quat[1] = (double)(cyawspitch * croll + syawcpitch * sroll);
   m_quat[2] = (double)(syawcpitch * croll - cyawspitch * sroll);
}


// Get Euler angles.
void CQuaternion::GetEulerAngles(double& pitch, double& yaw, double& roll)
{
   double r11, r21, r31, r32, r33, r12, r13;
   double q00, q11, q22, q33;
   double tmp;

   q00 = m_quat[3] * m_quat[3];
   q11 = m_quat[0] * m_quat[0];
   q22 = m_quat[1] * m_quat[1];
   q33 = m_quat[2] * m_quat[2];

   r11 = q00 + q11 - q22 - q33;
   r21 = 2 * (m_quat[0] * m_quat[1] + m_quat[3] * m_quat[2]);
   r31 = 2 * (m_quat[0] * m_quat[2] - m_quat[3] * m_quat[1]);
   r32 = 2 * (m_quat[1] * m_quat[2] + m_quat[3] * m_quat[0]);
   r33 = q00 - q11 - q22 + q33;

   tmp = fabs(r31);
   if (tmp > 0.999999)
   {
      r12 = 2 * (m_quat[0] * m_quat[1] - m_quat[3] * m_quat[2]);
      r13 = 2 * (m_quat[0] * m_quat[2] + m_quat[3] * m_quat[1]);

      pitch = RadiansToDegrees(0.0f);
      yaw   = RadiansToDegrees((double)(-(pi / 2) * r31 / tmp));
      roll  = RadiansToDegrees((double)atan2(-r12, -r31 * r13));
   }
   else
   {
      pitch = RadiansToDegrees((double)atan2(r32, r33));
      yaw   = RadiansToDegrees((double)asin(-r31));
      roll  = RadiansToDegrees((double)atan2(r21, r11));
   }
}


// Clone quaternion.
CQuaternion *CQuaternion::Clone()
{
   CQuaternion *q = new CQuaternion();

   ASSERT(q != NULL);
   q->m_quat[0] = m_quat[0];
   q->m_quat[1] = m_quat[1];
   q->m_quat[2] = m_quat[2];
   q->m_quat[3] = m_quat[3];
   return (q);
}


// Load quaternion.
void CQuaternion::Serialize(Stream& fp)
{
   for (int i = 0; i < 4; i++)
   {
      FREAD_FLOAT(&m_quat[i], fp);
   }
   FREAD_INT(&m_normal_count, fp);
}


// Save quaternion.
void CQuaternion::Store(Stream& fp)
{
   for (int i = 0; i < 4; i++)
   {
      FWRITE_FLOAT(&m_quat[i], fp);
   }
   FWRITE_INT(&m_normal_count, fp);
}
