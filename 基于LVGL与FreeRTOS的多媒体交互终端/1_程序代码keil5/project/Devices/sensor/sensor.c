#include "sensor.h"
#include "sensor_iic.h"
#include "sensor_iic_port.h"
#include "qmc5883.h"
#include "icm42688.h"
#include "sht30.h"
#include "bh1750.h"
#include "../../system/delay/delay.h"
#include "../../system/debug/debug.h"
#include <math.h>

void imu_euler_angle(short a[3], short g[3], float euler_angle[3]);


void sensor_init(void)
{
	uint8_t iic_addr[6] = {0};
	
	sensor_iic_init(); 
	
	sensor_addr_scan(iic_addr, 6);
	for(int i = 0; i < 6; i++)
	{
		if(iic_addr[i] != 0) printf("[info] sensor iic addr = 0x%x\r\n", iic_addr[i]);
	}
	
	qmc5883_init();
	sht30_init();
	bh1750_init();
	icm42688_init();
	
}



/*-------------------------------------------------------------
                       数据处理
--------------------------------------------------------------*/
uint8_t sensor_get_imu(float *pitch, float *roll, float *yaw)
{
	short a[3]; short g[3]; float euler_angle[3];
	
	icm42688_get_accelerometer(&a[0], &a[1], &a[2]);
	icm42688_get_gyroscope(&g[0], &g[1], &g[2]);
	imu_euler_angle(a, g, euler_angle);
	
	*pitch = euler_angle[2];	// 绕着y轴
	*roll  = euler_angle[1];	// 绕着x轴
	*yaw   = euler_angle[0];	// 绕着z轴
	printf("%f %f %f\r\n", euler_angle[2], euler_angle[1], euler_angle[0]);
	return 0;
}

uint8_t sensor_get_compass(float *g)
{
	*g = 1.0;
	return 0;
}


uint8_t sensor_get_temperature_humidity(float *temperature, float *humidity)
{
	sht30_get_temperature_humidity(temperature, humidity);
	
	return 0;
}

uint8_t sensor_get_light(float *light)
{
	float value = bh1750_single_read_light(BH1750_ONE_H_RES_MODE2);
	
	*light = value;
	return 0;
}







/*-------------------------------------------------------------
                       欧拉角计算
--------------------------------------------------------------*/
#define Acc_Gain 	(16.0f 	 / 32768)				// 加速度转换为（m/s^2）	(初始化加速度计量程±16g，	  	16g / 32768)
#define Gyro_Gain 	(2000.0f / 32768)				// 陀螺仪转化为（角度/秒）	(初始化陀螺仪传感器±2000dps 	2000dps / 32768)
#define Gyro_Gr 	(3.1415f / 180 * Gyro_Gain)		// 角度/秒转换成弧度/秒 	(3.1415 / 180 * Gyro_Gain)
#define G 			9.80665f						// 重力加速度
#define IMU_TIMES	0.05f 							// 计算周期的50ms(单位s)
#define Kp 			1.50f							// 欧拉角计算参数
#define Ki 			0.005f							// 欧拉角计算参数
static void imu_calculate_euler_angle(float accel[3], float gyro[3], float euler_angle[3]);
static float invSqrt(float x);


void imu_euler_angle(short a[3], short g[3], float euler_angle[3])
{
	float gyro[3],acce[3];
	
	// 将加速度值转换为m/s^2
	acce[0] = (float)a[0] * Acc_Gain * G;
	acce[1] = (float)a[1] * Acc_Gain * G;
	acce[2] = (float)a[2] * Acc_Gain * G;
	
	// 将陀螺仪值转换为 弧度/s
	gyro[0] = (float)g[0] * Gyro_Gr;
	gyro[1] = (float)g[1] * Gyro_Gr;
	gyro[2] = (float)g[2] * Gyro_Gr;
	
	imu_calculate_euler_angle(acce, gyro, euler_angle);
	
}


//计算欧拉角
static void imu_calculate_euler_angle(float acce[3], float gyro[3],float euler_angle[3])
{
	static float q0 = 1, q1 = 0, q2 = 0, q3 = 0;	
	static float exInt = 0, eyInt = 0, ezInt = 0;
	
	float vx,vy,vz;	//实际重力加速度
	float ex,ey,ez;	//叉积计算的误差
	float norm;
	float gx = gyro[0], gy = gyro[1], gz = gyro[2];
	float ax = acce[0], ay = acce[1], az = acce[2];
 	float q0q0 = q0*q0;
 	float q0q1 = q0*q1;
	float q0q2 = q0*q2;
	float q0q3 = q0*q3;
	float q1q1 = q1*q1;
 	float q1q2 = q1*q2;
 	float q1q3 = q1*q3;
	float q2q2 = q2*q2;
	float q2q3 = q2*q3;
	float q3q3 = q3*q3;
	float halfT = (float)IMU_TIMES / 2;
	
	if(ax*ay*az == 0)
		return;
	
	//加速度计测量的重力方向(机体坐标系)
	norm = invSqrt(ax*ax + ay*ay + az*az);
	ax = ax * norm;
	ay = ay * norm;
	az = az * norm;
	
	//四元数推出的实际重力方向(机体坐标系)
	vx = 2*(q1q3 - q0q2);												
  	vy = 2*(q0q1 + q2q3);
  	vz = q0q0 - q1q1 - q2q2 + q3q3;
	
	//叉积误差
	ex = (ay*vz - az*vy);
	ey = (az*vx - ax*vz);
	ez = (ax*vy - ay*vx);
	
	//叉积误差积分为角速度
	exInt = exInt + ex * Ki;
	eyInt = eyInt + ey * Ki;
	ezInt = ezInt + ez * Ki;
	
	//角速度补偿
	gx = gx + Kp*ex + exInt;
	gy = gy + Kp*ey + eyInt;
	gz = gz + Kp*ez + ezInt;
	
	//更新四元数
  	q0 = q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
  	q1 = q1 + (q0*gx + q2*gz - q3*gy)*halfT;
  	q2 = q2 + (q0*gy - q1*gz + q3*gx)*halfT;
  	q3 = q3 + (q0*gz + q1*gy - q2*gx)*halfT;	
	
	//单位化四元数
  	norm = invSqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
  	q0 = q0 * norm;
  	q1 = q1 * norm;
  	q2 = q2 * norm;  
  	q3 = q3 * norm;
	
	//四元数反解欧拉角
	euler_angle[0] = atan2(2.f * (q1q2 + q0q3), q0q0 + q1q1 - q2q2 - q3q3) * 57.3f;
	euler_angle[1] = -asin(2.f * (q1q3 - q0q2)) * 57.3f;
	euler_angle[2] = atan2(2.f * q2q3 + 2.f * q0q1, q0q0 - q1q1 - q2q2 + q3q3)* 57.3f;
}


//快速计算 1/Sqrt(x)
static float invSqrt(float x) 
{
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));
	return y;
}


