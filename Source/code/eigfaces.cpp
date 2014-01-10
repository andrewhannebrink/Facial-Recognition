
/////////////////////////////////////////////////////////////////////////////////////////////////
//	Project 4: Eigenfaces                                                                      //
//  CSE 455 Winter 2003                                                                        //
//	Copyright (c) 2003 University of Washington Department of Computer Science and Engineering //
//                                                                                             //
//  File: eigfaces.cpp                                                                         //
//	Author: David Laurence Dewey                                                               //
//	Contact: ddewey@cs.washington.edu                                                          //
//           http://www.cs.washington.edu/homes/ddewey/                                        //
//                                                                                             //
/////////////////////////////////////////////////////////////////////////////////////////////////



#include "stdafx.h"


EigFaces::EigFaces()
:
Faces()
{
	//empty
}

EigFaces::EigFaces(int count, int width, int height)
:
Faces(count, width, height)
{
	//empty
}

void EigFaces::projectFace(const Face& face, Vector& coefficients) const
{
	if (face.getWidth()!=width || face.getHeight()!=height) {
		throw Error("Project: Face to project has different dimensions");
	}

	coefficients.resize(getSize());
	// ----------- TODO #2: compute the coefficients for the face and store in coefficients.
	Face dif (width, height);
	face.sub(average_face, dif);
	for (int i = 0; i < getSize(); i++) {
		coefficients[i] = dif.dot((*this)[i]);
	}

}

void EigFaces::constructFace(const Vector& coefficients, Face& result) const
{	
	// ----------- TODO #3: construct a face given the coefficients
	result = average_face;
	std::vector<Face> eVectors;
	eVectors.resize(getSize());
	for (int i = 0; i < getSize(); i++){
		Face temp;
		temp = (*this)[i];
		temp *= coefficients[i];
//		eVectors[i] *= coefficients[i];
		result.add(temp, result);
		/*Vector v = (*this)[i];
		v *= coefficients[i];
		result.add(v, result);*/
		//result += (*this)[i];

	/*	Face temp (width, height);
		for (int j = 0; j < temp.size; j++) {
			std::cout << j <<std::endl;
			temp[j] = coefficients[j]*(*this)[i][j];
		}*/
		//result += temp;
	}

}

bool EigFaces::isFace(const Face& face, double max_reconstructed_mse, double& mse) const
{
	// ----------- TODO #4: Determine if an image is a face and return true if it is. Return the actual
	// MSE you calculated for the determination in mse
	// Be sure to test this method out with some face images and some non face images
	// to verify it is working correctly.
	//	return false;  // placeholder, replace
		Vector coefs;
		Face proj = face;
		projectFace(face, coefs);
		constructFace(coefs, proj);
		mse = proj.mse(face);
		if (mse < max_reconstructed_mse) {
			return true;
		}
		else {
			return false;
		}
}

bool EigFaces::verifyFace(const Face& face, const Vector& user_coefficients, double max_coefficients_mse, double& mse) const
{
	// ----------- TODO #5 : Determine if face is the same user give the user's coefficients.
	// return the MSE you calculated for the determination in mse.
	//return false;  // placeholder, replace
	Vector coefs;
	Face proj = face;
	projectFace(proj, coefs);
	mse = coefs.mse(user_coefficients);
	if (max_coefficients_mse < mse) {
		return false;
	}
	else {
		return true;
	}
}

void EigFaces::recognizeFace(const Face& face, Users& users) const
{
	// ----------- TODO #6: Sort the users by closeness of match to the face
	
	Vector coefs;
	Face proj = face;
	projectFace(proj,coefs);
	for (int i = 0; i < users.getSize(); i++) {
		double mse = users[i].mse(coefs);
		//std::cout << mse << std::endl;
		users[i].setMse(mse);
	}
	users.sort();

		
	
}

void EigFaces::findFace(const Image& img, double min_scale, double max_scale, double step, int n, bool crop, Image& result) const
{
	// ----------- TODO #7: Find the faces in Image. Search image scales from min_scale to max_scale inclusive,
	// stepping by step in between. Find the best n faces that do not overlap each other. If crop is true,
	// n is one and you should return the cropped original img in result. The result must be identical
	// to the original besides being cropped. It cannot be scaled and it must be full color. If crop is
	// false, draw green boxes (use r=100, g=255, b=100) around the n faces found. The result must be
	// identical to the original image except for the addition of the boxes.
	std::vector<FacePosition> topMatches (n);
	FacePosition topMatch;
	std::cout << "n: " << n << std::endl;
	//initialize values for topMatches
	for (int i = 0; i < n; i++) {
		FacePosition pn;
		pn.error = DBL_MAX;
		pn.x = 5000000;
		pn.y = 5000000;
		topMatches[i] = pn;
	}
	std::cout << "initsize: " << topMatches.size() << std::endl;
	double cur_scale = min_scale;
	Face reconstruction;
	Face sub (width, height);
	Vector coefs;
	double max_mse = DBL_MAX;
	while (cur_scale <= max_scale) {
		
		Image scaImg (int(img.getWidth()*cur_scale), int(img.getHeight()*cur_scale), img.getColors());
		img.resample(scaImg);
		for (int x = 0; x < scaImg.getWidth(); x++) {
			for (int y = 0; y < scaImg.getHeight(); y++) {
	   		
//				std::cout << "scaImg: " << scaImg.getWidth() << " , " << scaImg.getHeight() << std::endl;
//
//				std::cout << "sub: " << sub.getWidth() << ", " << sub.getHeight() << std::endl;
//				std::cout << "max_mse: " << max_mse << std::endl;

				sub.subimage(x, x + sub.getWidth()-1, y, y + sub.getHeight()-1, scaImg, false);	
		//		projectFace(sub, coefs);
		//		constructFace(coefs, reconstruction);
				double mse;
			//	double mse = sub.mse(reconstruction)*sub.mse(average_face)/sub.var();
				bool itIsAFace = isFace(sub, max_mse, mse);
				//std::cout <<"mse: "<< mse << std::endl;
				if (crop) {
					if (itIsAFace) {
						if (mse < topMatch.error) {
							topMatch.x = x;
							topMatch.y = y;
							topMatch.error = mse;
							topMatch.scale = cur_scale;
						}
					}
					img.crop(topMatch.x+width/2, topMatch.y+height/2, topMatch.x+5*width/2, topMatch.y+5*height/2, result);
				}
				if (!crop) {
					for (int i = 0; i < n; i++) {
						if (mse < topMatches[i].error){
							//STOP IF TOO CLOSE
							bool shouldStop = false;
							//BROKEN REMNANTS OF OVERLAP PREVENTION
							for (int j = 0; j<n; j++) {
								//std::cout <<abs(abs(x/cur_scale-topMatches[j].x/topMatches[j].scale)) << std::endl;
								//if (abs(topMatches[j].x/topMatches[j].scale -x/cur_scale)<50 && abs(topMatches[j].y/topMatches[j].scale-y/cur_scale)<50) {
								if (abs(topMatches[j].x - x) < 50 && abs(topMatches[j].y - y) < 50) {
									shouldStop = true;
									break;
								}
							}
							if (shouldStop==true) {
								break;
							}
							int j = n-1;
							while (i<=j) {
								if (j==i) {
									topMatches[i].x = x;
									topMatches[i].y = y;
									topMatches[i].error = mse;
									topMatches[i].scale = cur_scale;
								} else {
									topMatches[j] = topMatches[j-1];
								}
								//for (int i = 0; i < topMatches.size(); i++) {
								//	std::cout << topMatches[i].error << ",";
								//}
								//std::cout <<std::endl;
								j--;
							}
							if (j < i) {
								break;
							}
						}
					}
				}
			}
		}
		//for (int i = 0; i < topMatches.size(); i++){
		//	std::cout << topMatches[i].error <<", "<<  std::endl;

		//}
		/*Face resultFace(width,height);
		resultFace.subimage(bestFacePos.x,bestFacePos.x+width,bestFacePos.y,bestFacePos.y+height,scaImg,false);*/


		cur_scale += step;
	}
	if (!crop) {	
		result = img;
		std::cout << "n at end = " << n <<std::endl;
		for (int i = 0; i < n; i++) {
			FacePosition p = topMatches[i];
			int x1 = p.x+1*int(width/p.scale);
			int y1 = p.y+1*int(height/p.scale);
			int x2 = p.x+2*int(width/p.scale);
			int y2 = p.y+2*int(height/p.scale);
			result.line(x1,y1,x2,y1,100,255,100);
			result.line(x1,y1,x1,y2,100,255,100);
			result.line(x1,y2,x2,y2,100,255,100);
			result.line(x2,y2,x2,y1,100,255,100);
						 
		/*	result.line(p.x, p.y, p.x+int(width/p.scale), p.y, 100, 255, 100);
			result.line(p.x, p.y+int(height/p.scale), p.x+int(width/p.scale), p.y+int(height/p.scale), 100, 255, 100);
			result.line(p.x, p.y, p.x, p.y+int(height/p.scale), 100, 255, 100);
			result.line(p.x+int(width/p.scale), p.y, p.x+int(width/p.scale), p.y+int(height/p.scale), 100, 255, 100);
			std::cout << "heeeeerrrrrreeeee n = " << n << std::endl;
			std::cout << " i = " << i << std::endl;*/
		}
	}
	else {
		img.crop(topMatch.x+width/2, topMatch.y+height/2, topMatch.x+5*width/2, topMatch.y+5*height/2, result);
	}
/*	std::cout << topMatches.size() << std::endl;
	FacePosition bestFacePos = topMatches[3];
	if (crop == true) {
		std::cout << "w: " << width << "h: " << height << std::endl;
		img.crop(bestFacePos.x+width/2, bestFacePos.y+height/2, bestFacePos.x+5*width/2, bestFacePos.y+5*height/2, result);
		std::cout <<"HELLLLOOOOOOOOOOOOO" << std::endl;
	}*/


}

void EigFaces::morphFaces(const Face& face1, const Face& face2, double distance, Face& result) const
{
	// TODO (extra credit): MORPH along *distance* fraction of the vector from face1 to face2 by
	// interpolating between the coefficients for the two faces and reconstructing the result.
	// For example, distance 0.0 will approximate the first, while distance 1.0 will approximate the second.
	// Negative distances are ok two.

}

const Face& EigFaces::getAverage() const
{
	return average_face;
}

void EigFaces::setAverage(const Face& average)
{
	average_face=average;
}



