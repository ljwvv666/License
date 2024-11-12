#include "AnnPredict.h"

string AnnPredict::ZHCHARS[] = { "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "³", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "ԥ", "��", "��", "��", "��", "��" };
char AnnPredict::CHARS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };

AnnPredict::AnnPredict(const char* ann_model, const char* ann_zh_model)
{
	ann = ANN_MLP::load(ann_model);
	ann_zh = ANN_MLP::load(ann_zh_model);
	annHog = new HOGDescriptor(Size(32, 32), Size(16, 16), Size(8, 8), Size(8, 8), 3);
}

AnnPredict::~AnnPredict()
{
	ann->clear();
	ann.release();
	ann_zh->clear();
	ann_zh.release();
	if (annHog) {
		delete annHog;
		annHog = 0;
	}
}

string AnnPredict::doPredict(Mat final_plate)
{
	//Ԥ����
	//�ҶȻ�
	Mat gray;
	cvtColor(final_plate, gray, COLOR_BGR2GRAY);
	//��ֵ��
	Mat shold;
	threshold(gray, shold, 0, 255, THRESH_OTSU + THRESH_BINARY);
	//imshow("��ֵ������", shold);
	
	if (!clearMaoDing(shold)) {
		return string("δʶ�𵽳���");
	}
	//imshow("ȥí����", shold);

	//���ַ�����
	vector<vector<Point>> contours;
	findContours(
		shold, //����ͼ��
		contours, //���ͼ��
		RETR_EXTERNAL, //ȡ�������
		CHAIN_APPROX_NONE //ȡ���������е����ص�
	);
	RotatedRect rotatedRect;
	vector<Rect> vec_ann_rects;
	for each (vector<Point> points in contours)
	{
		Rect rect = boundingRect(points); //ȡ��С��Ӿ��Σ�����ת/���Ƕȣ�
		//rectangle(final_plate, rect, Scalar(0, 0, 255)); //����ɫ����
		Mat m = shold(rect);
		if (verifyCharSize(m)) {
			vec_ann_rects.push_back(rect);
		}
	}
	//cout << "size: " << vec_ann_rects.size() << endl;
	//for each (Rect rect in vec_ann_rects)
	//{
	//	rectangle(final_plate, rect, Scalar(0, 255, 0)); //����ɫ����
	//}
	//imshow("�ַ�����", final_plate);

	//���򣺴�����
	sort(vec_ann_rects.begin(), vec_ann_rects.end(), [](const Rect& rect1, const Rect& rect2) {
		return rect1.x < rect2.x;
	});

	//��ȡ�����ַ��������
	int cityIndex = getCityIndex(vec_ann_rects);

	//���ƺ����ַ����λ��
	Rect chineseRect;
	getChineseRect(vec_ann_rects[cityIndex], chineseRect);

	vector<Mat> plateCharMats; //����7���ַ�ͼ��
	plateCharMats.push_back(shold(chineseRect)); //���������ַ�
	if (vec_ann_rects.size() < 6) {
		return string("δʶ�𵽳���");
	}
	int count = 6;
	for (int i = cityIndex; i < vec_ann_rects.size() && count; i++, count--)
	{
		plateCharMats.push_back(shold(vec_ann_rects[i])); //������м�֮�������ַ�
	}

	char winName[100];
	for (int i = 0; i < plateCharMats.size(); i++)
	{
		sprintf(winName, "$d �ַ�", i);
		imshow(winName, plateCharMats[i]);
	}

	string str_plate;
	predict(plateCharMats, str_plate);

	return str_plate;
}

//ÿ������ֵ�ı仯����
//���í����&�ж��Ƿ��ʺϺ���ķָ��ַ�
bool AnnPredict::clearMaoDing(Mat& src) {
	int maxChangeCount = 12;
	vector<int> changes; //ͳ��ÿ�еı仯����
	for (int i = 0; i < src.rows; i++)
	{
		int changeCount = 0; //��¼��i�еı仯����
		for (int j = 0; j < src.cols - 1; j++)
		{
			char pixel_1 = src.at<char>(i, j); //ǰ����
			char pixel_2 = src.at<char>(i, j + 1); //������
			if (pixel_1 != pixel_2) {
				//���ط����˸ı�
				changeCount++;
			}
		}
		changes.push_back(changeCount);
	}

	//�����ַ��������������
	//��������������Լ�����ַ��ĸ߶�
	int charRows = 0;
	for (int i = 0; i < src.rows; i++)
	{
		if (changes[i] >= 12 && changes[i] <= 60) {
			//��ǰ��i�����ڳ����ַ�����
			charRows++;
		}
	}

	//�ж��ַ�ռ��
	float heightPercent = charRows * 1.0 / src.rows; //�ַ��߶�ռ��
	if (heightPercent <= 0.4) {
		return false; //�����ַ��߶�ռ�Ȳ�����Ҫ��
	}

	//�ַ����� Լ���ڰ�ɫ���صĸ���
	float src_area = src.rows * src.cols;
	float areaPercent = countNonZero(src) * 1.0 / src_area;
	if (areaPercent <= 0.2 || areaPercent >= 0.5) {
		return false; //�����ַ����ռ�Ȳ���Ҫ��
	}

	for (int i = 0; i < changes.size(); i++)
	{
		int changeCount = changes[i];
		if (changeCount < maxChangeCount) {
			//����í�����������У�ȫ�� Ĩ�ڣ��޸�����ֵΪ0��
			for (int j = 0; j < src.cols; j++)
			{
				src.at<char>(i, j) = 0;
			}
		}
	}
	return true;
}

bool AnnPredict::verifyCharSize(Mat src)
{
	//�����߱�
	float aspect = 0.5f;
	//��ʵ��߱�
	float realAspect = float(src.cols) / float(src.rows);
	//�ַ���С�߶�
	float minHeight = 10.0f;
	float maxHeight = 35.0f;

	float error = 0.7f; //�ݴ���
	float maxAspect = aspect + aspect * error;
	float minAspect = 0.05f;

	float src_area = src.rows * src.cols;
	float areaPercent = countNonZero(src) * 1.0 / src_area;

	if (src.rows >= minHeight && src.rows <= maxHeight //�߶�
		&& realAspect >= minAspect && realAspect <= maxAspect //��߱�
		&& areaPercent <= 0.8f) {
		return true;
	}

	return false;
}

//��ȡ�����ַ��������ο���show/get_city_character_throry.png
int AnnPredict::getCityIndex(vector<Rect> rects)
{
	int cityIndex = 0;
	for (int i = 0; i < rects.size(); i++)
	{
		Rect rect = rects[i];
		int midX = rect.x + rect.width / 2;
		if (midX < 136 / 7 * 2 && midX > 136 / 7) {
			cityIndex = i;
			break;
		}
	}
	return cityIndex;
}

void AnnPredict::getChineseRect(Rect cityRect, Rect& chineseRect)
{
	//��ȣ��ȳ����ַ���һ��
	float width = cityRect.width * 1.15f;
	int cityX = cityRect.x; //�����ַ���x
	int x = cityX - width;
	chineseRect.x = x >= 0 ? x : 0;
	chineseRect.y = cityRect.y;
	chineseRect.width = width;
	chineseRect.height = cityRect.height;
}

void AnnPredict::predict(vector<Mat> plateCharMats, string& str_plate)
{
	for (int i = 0; i < plateCharMats.size(); i++)
	{
		Mat plate_char = plateCharMats[i];
		Mat features;
		getHogFeatures(annHog, plate_char, features);

		Mat sample = features.reshape(1, 1);
		Mat response;
		Point maxLoc;
		Point minLoc;

		if (i) { //��0��true
			//��ĸ + ����
			ann->predict(sample, response);
			minMaxLoc(response, 0, 0, &minLoc, &maxLoc);
			int index = maxLoc.x; //��������ֵ(CHAR���������ֵ��
			str_plate += CHARS[index];
		}
		else { //����
			ann_zh->predict(sample, response);
			minMaxLoc(response, 0, 0, &minLoc, &maxLoc);
			int index = maxLoc.x; //��������ֵ(ZHCHAR���������ֵ��
			str_plate += ZHCHARS[index];
		}
		
	}
}

void AnnPredict::getHogFeatures(HOGDescriptor* hog, Mat src, Mat& dst)
{
	//��һ��
	Mat trainImg = Mat(hog->winSize, CV_32S);
	resize(src, trainImg, hog->winSize);

	//��������
	vector<float> descriptor;
	hog->compute(trainImg, descriptor, hog->winSize);

	Mat feature(descriptor);
	feature.copyTo(dst);

	feature.release();
	trainImg.release();
}
