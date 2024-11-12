#include "SobelLocate.h"

SobelLocate::SobelLocate()
{
}

SobelLocate::~SobelLocate()
{
}

/**
* sobel ��λ
*/
void SobelLocate::locate(Mat src, vector<Mat>& dst_plates)
{
	//imshow("src", src);
	//1.��˹ģ��
	Mat blur;
	//��opencv�У�ֻ֧�������뾶
	GaussianBlur(src, blur, Size(5, 5), 0);
	//imshow("blur", blur);

	//2.�ҶȻ�
	Mat gray;
	cvtColor(blur, gray, COLOR_BGR2GRAY);
	//imshow("gray", gray);

	//3.Sobel����
	Mat sobel_16;
	//Sobel�󵼺�ĵ���ֵ����С��0���ߴ���255,8λ�治��
	Sobel(gray, sobel_16, CV_16S, 1, 0);
	//imshow("sobel_16", sobel_16);
	//16λת��8λ
	Mat sobel;
	convertScaleAbs(sobel_16, sobel);
	//imshow("sobel", sobel);

	//4.��ֵ��(��/��)
	//����ֵ�� �ʺϣ����ƣ����ǳ�֣�
	//����ֵ�� �ʺϣ�ǳ������
	Mat shold;
	threshold(sobel, shold, 0, 255, THRESH_OTSU + THRESH_BINARY);
	//imshow("shold", shold);

	//5.��̬ѧ�������ղ�����
	Mat element = getStructuringElement(MORPH_RECT, Size(17, 3)); //����ֵ���ɵ�
	Mat close;
	morphologyEx(shold, close, MORPH_CLOSE, element);
	//imshow("close", close);

	//6.������
	vector<vector<Point>> contours;
	findContours(
		close, //����ͼ��
		contours, //�������
		RETR_EXTERNAL, //ȡ������
		CHAIN_APPROX_NONE //ȡ�������������ص�
	);
	RotatedRect rotatedRect;
	vector<RotatedRect> vec_sobel_rects;
	for each (vector<Point> points in contours)
	{
		rotatedRect = minAreaRect(points); //ȡ��С��Ӿ��Σ�����ת/���Ƕȣ�
		//rectangle(src, rotatedRect.boundingRect(), Scalar(0, 0, 255)); //����ɫ����

		//7.�ߴ��ж�
		//�������˲�����Ҫ��ľ���
		if (verifySizes(rotatedRect)) {
			vec_sobel_rects.push_back(rotatedRect);
		}
	}

	//for each (RotatedRect rect in vec_sobel_rects)
	//{
	//	rectangle(src, rect.boundingRect(), Scalar(0, 255, 0)); //����ɫ����
	//}

	//���ν���
	tortuosity(src, vec_sobel_rects, dst_plates);
	//for each (Mat m in dst_plates)
	//{
	//	/*imshow("sobel ��ѡ����", m);
	//	waitKey();*/
	//}
	//imshow("sobelȡ����", src);

	blur.release();
	gray.release();
	sobel_16.release();
	sobel.release();
	shold.release();
	element.release();
	close.release();
}
