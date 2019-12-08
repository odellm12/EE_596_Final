#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cv2
import glob
import matfaceDrawlib.pyfaceDraw as plt
import matfaceDrawlib.patches as patches
import numpy as np
import os
import sys
import csv
from keras.models import Model, Sequential, load_model
from keras.layers import Input, Dense
from keras.utils.np_utils import to_categorical
from keras.layers.normalization import BatchNormalization
from sklearn.metrics import confusion_matrix
from keras.preprocessing.image import ImageDataGenerator
from math import floor
from PIL import Image
np.random.seed(1337)



def neuralNetworkFilter(imageW_SubSample, imageH_SubSample):
	#a linear stack of layers
	neuralNetModel = Sequential()
	#add a layer
	neuralNetModel.add(BatchNormalization(input_shape=(imageW_SubSample*imageH_SubSample,)))
	neuralNetModel.add(Dense(128, activation="relu"))
	neuralNetModel.add(BatchNormalization())
	neuralNetModel.add(Dense(64, activation="relu"))
	neuralNetModel.add(BatchNormalization())
	neuralNetModel.add(Dense(32, activation="relu"))
	neuralNetModel.add(BatchNormalization())
	neuralNetModel.add(Dense(2, activation="softmax"))
	neuralNetModel.compile(optimizer='adagrad', loss='categorical_crossentropy', metrics=["accuracy"])
	return neuralNetModel

def trainingDataProcesser(facesData, noFacesData, pos_ini, size):
	files = glob.glob ("C:/Users/aminegar/AppData/Local/Programs/Python/Python37/Scripts/lll/*.jpg")
	faces = np.empty((0,576), int)
	for myFile in files:
		img = Image.open(myFile).convert('LA')
		img = img.resize((24, 24), Image.ANTIALIAS)
		arr = (np.array(img))
		shape = arr.shape
		temp = []
		for y in range(24):
			for x in range(24):
				temp.append(arr[x,y][0])
		faces = np.vstack((faces, temp))
		print('face shape:', np.array(faces).shape)
		
	files = glob.glob ("C:/Users/aminegar/AppData/Local/Programs/Python/Python37/Scripts/ggg/*.jpg")
	no_faces = np.empty((0,576), int)
	for myFile in files:
		#print(myFile)
		img = Image.open(myFile).convert('LA')
		img = img.resize((24, 24), Image.ANTIALIAS)
		arr = (np.array(img))
		shape = arr.shape
		temp = []
		for y in range(24):
			for x in range(24):
				temp.append(arr[x,y][0])
		no_faces = np.vstack((no_faces, temp))
		print('noface shape:', np.array(no_faces).shape)		
				
	# genfromtxt runs two main loops. The first loop converts each line of the file in a sequence of strings. The second loop converts each string to the appropriate data type in an Array
	#faces      = np.genfromtxt(facesData, dtype = int, delimiter=",")
	#faces = data
	#print(faces)
	#Itirate through all facesData and add a 1 in y_faces for each row
	y_faces    = [1 for i in range(faces.shape[0])] #Num rows
	#no_faces   = np.genfromtxt(noFacesData, dtype = int, delimiter=",")
	#Itirate through all no_faces and add a 0 in y_no_faces for each row
	y_no_faces = [0 for i in range(no_faces.shape[0])]
	#Join a sequence of arrays along an existing axis
	X = np.concatenate((faces, no_faces), axis=0)
	Y = np.concatenate((y_faces, y_no_faces), axis=0)
	# slice items between indexes 
	X = X[pos_ini:size]
	Y = Y[pos_ini:size]
	return X, Y
	
def faceDraw(img, faces_xy):
	print(len(faces_xy))
	fig,ax = plt.subfaceDraws(1)
	ax.imshow(img)
	for i in range(len(faces_xy)):	
		width, height = faces_xy[i][3]-faces_xy[i][1], faces_xy[i][2]-faces_xy[i][0]
		rect = patches.Rectangle((faces_xy[i][0], faces_xy[i][1]),width, height,linewidth=0.5,edgecolor='b',facecolor='none')
		ax.add_patch(rect)
	plt.show()

def faceFinder(img_file, neuralNetModel, imageW_SubSample, imageH_SubSample, pyramidStepsW, pyramidStepsH, subRate, debug):
	im = Image.open(img_file)
	im_or = im.copy()
	w, h = im.size
	faces_xy = []
	i = 0
	while im.width>=imageW_SubSample and im.height>=imageH_SubSample:
		im.thumbnail((w, h), Image.ANTIALIAS)
		# Cropping #
		regions   = []
		positions = []
		for j in range(0, w, pyramidStepsW): #, imageW_SubSample):
			for p in range(0, h, pyramidStepsH):# imageH_SubSample):
				if debug: print((j, p, imageW_SubSample, imageH_SubSample))
				regions.append(im.crop((j, p, j+imageW_SubSample, p+imageH_SubSample)))
				regions[-1] = regions[-1].convert("L") # Grayscale #
				positions.append((j, p, j+imageW_SubSample, p+imageH_SubSample))
		# Face extraction #
		for j in range(len(regions)):
			region = np.array((regions[j].getdata()))
			print(region.shape)
			region = (region - np.mean(region)) / np.std(region)
			#from given a trained model, predict the label of a new set of data and retuns indices of the max element of the array 
			res = np.argmax(neuralNetModel.predict(np.array([region])))
			if res==1:
				#if debug: print("FIND", positions[j])
				faces_xy.append((positions[j][0]/(subRate**i), positions[j][1]/(subRate**i),
								 positions[j][2]/(subRate**i), positions[j][3]/(subRate**i)))
				#if debug: print("ORIG", faces_xy[-1])
		w = int(w*subRate)
		h = int(h*subRate)
		i += 1
	faceDraw(im_or, faces_xy)	
	
def train_model(neuralNetModel, faces_path, no_faces_path, pos_ini, size, epochs, neuralNetworkModelFile, debug=True, batch_size=32):
	X, Y = trainingDataProcesser(faces_path, no_faces_path, pos_ini, size)
	#to convert class vector (integers) to binary class matrix
	Y    = to_categorical(Y)
	# To Generate batches of tensor image data with real-time data augmentation. The data will be looped over (in batches)
	datagen = ImageDataGenerator(featurewise_center=True, 
								 samplewise_center=False, 
								 featurewise_std_normalization=True, 
								 samplewise_std_normalization=False,
								 rotation_range=20,
								 horizontal_flip=True)
	datagen.fit(X.reshape((X.shape[0], 1, 24, 24)))	 
	for e in range(epochs):
		batches = 0
		loss = 0
		
		for X_batch, Y_batch in datagen.flow(X.reshape((X.shape[0], 1, 24, 24)), Y, batch_size=batch_size):
		    # we need to break the loop by hand because
            # the generator loops indefinitely
			if batches >= floor(float(X.shape[0]) / batch_size)-1: break
			X_batch = X_batch.reshape((32, 576))
			#to Runs a single gradient update on a single batch of data.
			loss += neuralNetModel.train_on_batch(X_batch, Y_batch)[0]
			batches += 1
		avg_loss = loss / batches
		#Trains the model for a fixed number of epochs (iterations on a dataset)
		neuralNetModel.fit(X, Y, batch_size=batch_size, nb_epoch=1)
		#print("Epoch %d, loss %f\n" % (e, avg_loss))
	#to save a Keras model into a single HDF5 file which will contain:
	#	the architecture of the model, allowing to re-create the model
	#	the weights of the model
	#	the training configuration (loss, optimizer)
	#	the state of the optimizer, allowing to resume training exactly where you left off.
	neuralNetModel.save(neuralNetworkModelFile)
	return neuralNetModel
	
	
def modelEval(X_train, Y_train, X_dev, Y_dev, neuralNetModel, epochs):
	Y_train_cat = to_categorical(Y_train)
	Y_dev_cat   = to_categorical(Y_dev)
	neuralNetModel.fit(X_train, Y_train_cat, nb_epoch=epochs, debug=True)
	pred        = [np.argmax(p) for p in neuralNetModel.predict(X_dev)]
	C = confusion_matrix(Y_dev, pred)
	TN, FN, TP, FP = C[0][0], C[1][0], C[1][1], C[0][1]
	print("\n\n\t--- Confusion matrix ---\n")
	print("\t ¬Cara\t Cara\t\n")
	print("¬Cara\t", TN, "\t", FP, "\n")
	print(" Cara\t", FN, "\t", TP, "\n")
	print("\n* TN=%d\tFN=%d\tTP=%d\tFP=%d" % (TN, FN, TP, FP))
	
def randomiser(x, y):
	p = np.random.permutation(len(x))
	return x[p], y[p]
	
def main(args):
	neuralNetModel = neuralNetworkFilter(args.imageW_SubSample, args.imageH_SubSample)
	if args.functionMode=="modelTrain":
		neuralNetModel = train_model(neuralNetModel, args.faces_path, 
							   args.no_faces_path, args.pos_ini, 
							   args.size, args.epochs, args.neuralNetworkModelFile, bool(args.debug))
	elif args.functionMode=="modelTest":
		#Loads a model saved via save_model (neuralNetModel.save(neuralNetworkModelFile))
		neuralNetModel = load_model(args.neuralNetworkModelFile)
		faceFinder(args.test_file, neuralNetModel, args.imageW_SubSample, args.imageH_SubSample, args.pyramidStepsW, args.pyramidStepsH, args.subRate, bool(args.debug))
		
	
	elif args.functionMode=="modelEval":	
		X, Y = trainingDataProcesser(args.faces_path, args.no_faces_path, args.pos_ini, args.size)
		X, Y = randomiser(X, Y)
		X_train, Y_train = X[0:int((1-args.p_dev)*len(X))], Y[0:int((1-args.p_dev)*len(Y))]
		X_dev, Y_dev = X[int((1-args.p_dev)*len(X)):], Y[int((1-args.p_dev)*len(X)):]
		modelEval(X_train, Y_train, X_dev, Y_dev, neuralNetModel, args.epochs)
	else: exit()   

if __name__ == "__main__": 
	import argparse
	parser = argparse.ArgumentParser(description='Neural network based face detection')
	parser.add_argument('functionMode', action="store", type=str)
	parser.add_argument('--faces_path', action="store", type=str)
	parser.add_argument('--no_faces_path', action="store", type=str)
	parser.add_argument('--pos_ini', action="store", type=int)
	parser.add_argument('--size', action="store", type=int)
	parser.add_argument('--epochs', action="store", type=int)
	parser.add_argument('--neuralNetworkModelFile', action="store", type=str)
	parser.add_argument('--test_file', action="store", type=str)
	parser.add_argument('--imageW_SubSample', action="store", type=int)
	parser.add_argument('--imageH_SubSample', action="store", type=int)
	parser.add_argument('--pyramidStepsW', action="store", type=int)
	parser.add_argument('--pyramidStepsH', action="store", type=int)
	parser.add_argument('--p_dev', action="store", type=float)
	parser.add_argument('--subRate', action="store", type=float)
	parser.add_argument('--debug', action="store", type=int)
	main(parser.parse_args())
