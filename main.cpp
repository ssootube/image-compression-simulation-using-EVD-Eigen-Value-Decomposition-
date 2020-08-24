#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <Eigen/Dense>
#include <algorithm>
using namespace sf;
using namespace std;
using namespace Eigen;
typedef Eigen::Matrix< float, Eigen::Dynamic, Eigen::Dynamic > Mat;

void reset(Mat& mat) {
	//initialize matrix to zero matrix
	//행렬을 영행렬로 초기화합니다.
	for (int i = 0; i < mat.rows(); ++i)
		for (int j = 0; j < mat.cols(); ++j)
			mat(i, j) = 0;
}

bool compare(pair<double, Mat> a, pair<double, Mat> b) {
	//compare function for sorting eigen values in descending order
	//고유값을 내림차순으로 정렬하기 위한 비교함수
	return a.first * a.first > b.first * b.first;
}

class RGB {
public:
	enum { R, G, B };
	RGB() {
	}
	RGB(Mat red, Mat green, Mat blue) {
		size = red.rows();
		mat[R] = red;	mat[G] = green;	mat[B] = blue;
		empty = false;
	}
	Mat mat[3];
	int size;
	bool empty = true;


	vector<pair<double, Mat>> lists[3];
	bool processed[3] = { false,false,false };
	int added[3] = { -1,-1,-1 };
	void getEVDColor(int color, int rate) {
		if (!processed[color]) {
			SelfAdjointEigenSolver<Mat> eigensolver(mat[color]);
			if (eigensolver.info() != Success) abort();
			Mat eigenValues = eigensolver.eigenvalues();
			Mat eigenVectors = eigensolver.eigenvectors();
			eigenVectors = eigenVectors.householderQr().householderQ();//정규직교화
			int num = eigenValues.rows();//고유값의 개수

			for (int i = 0; i < num; ++i) {
				pair<double, Mat> element;
				element.first = eigenValues(i);
				element.second = eigenVectors.col(i);
				lists[color].push_back(element);
			}
			sort(lists[color].begin(), lists[color].end(), compare);
			processed[color] = true;
			mat[color] = Mat(size, size);
			reset(mat[color]);
			cout << endl;
		}
		if (added[color] <= rate) {
			for (int i = added[color] + 1; i < rate; ++i) {
				mat[color] += (lists[color][i].first * (lists[color][i].second * lists[color][i].second.transpose()));
			}
		}
		else {
			for (int i = added[color]; i >= rate; --i) {
				mat[color] -= (lists[color][i].first * (lists[color][i].second * lists[color][i].second.transpose()));
			}
		}
		added[color] = rate - 1;
	}
};

RGB getMatrixFromImage(Image& img) {
	//only square image available
	//it uses upper triangle part of the image only.
	if (img.getSize().x != img.getSize().y) return RGB();
	int size = img.getSize().x;

	Mat red(size, size);
	Mat green(size, size);
	Mat blue(size, size);
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			if (j < i) {
				red(i, j) = img.getPixel(i, j).r;
				green(i, j) = img.getPixel(i, j).g;
				blue(i, j) = img.getPixel(i, j).b;
			}
			else if (i == j) red(i, j) = 0;
			else {
				red(i, j) = img.getPixel(j, i).r;
				green(i, j) = img.getPixel(j, i).g;
				blue(i, j) = img.getPixel(j, i).b;
			}
		}
	}
	return RGB(red, green, blue);
}

Image getImageFromRGB(RGB& rgb) {
	Image  result;
	result.create(rgb.size, rgb.size);
	for (int i = 0; i < rgb.size; ++i) {
		for (int j = 0; j < rgb.size; ++j) {
			Color color;
			color.r = rgb.mat[RGB::R](i, j);
			color.g = rgb.mat[RGB::G](i, j);
			color.b = rgb.mat[RGB::B](i, j);
			result.setPixel(i, j, color.toInteger() < 0 ? Color(0) : color);
		}
	}
	return result;
}

int main() {

	string input;
	cout << "input image file name including extension>";
	cin >> input;

	Image original;
	original.loadFromFile(input);
	if (original.getSize().x != original.getSize().y) {
		cout << "only square image available";
		return -1;
	}

	RenderWindow window(VideoMode(original.getSize().x, original.getSize().y), "Math channel Ssootube Eigen Value Decomposition");
	RGB mat;
	mat = getMatrixFromImage(original);
	Image applied = getImageFromRGB(mat);
	Texture t; t.loadFromImage(applied);
	Sprite output(t);
	int k = 0;
	cout << "only upper triangular part of the image will be used." << endl;
	while (window.isOpen()) {
		Event e;
		while (window.pollEvent(e)) {
			if (e.type == Event::Closed)
				window.close();
			if (e.type == Event::KeyPressed)
				if (e.key.code == Keyboard::Enter)
				{
					window.clear();
					window.draw(output);
					window.display();
					if (k <= mat.size) {
						mat.getEVDColor(RGB::R, k);
						mat.getEVDColor(RGB::G, k);
						mat.getEVDColor(RGB::B, k++);
					}
					applied = getImageFromRGB(mat);
					t.loadFromImage(applied);
					output.setTexture(t);
					cout << "press enter to continue. current rank:" << ((k - 2 == -1) ? 0 : k - 2) << endl;
					break;
				}
		}
		
	}
	return 0;
}
