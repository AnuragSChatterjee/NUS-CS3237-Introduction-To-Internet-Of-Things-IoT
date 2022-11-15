# Importing the libraries
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.naive_bayes import GaussianNB
import pickle
import requests
import json
import sqlite3

# Importing the dataset

con=sqlite3.connect("deceptiondata.db")
cursor=con.cursor()
df = pd.read_sql_query("SELECT * FROM DECEPTION", con)

#dataset=cursor.execute("SELECT * FROM DECEPTION").fetchall()
con.commit()
con.close()
col_names = ["AverageTemperatureDif","MaxMinTemperatureDif","AverageHumidityDif","MaxMinHumidityDif","AveragePulseDif","MaxMinPulseDif","WordCount","NounCount","VerbCount","AdjectiveCount","Time","Truth"]
df.columns = col_names
#print(dataset.head()) #prints dataset
#maybe I used y wrong it should be the data.target
#print(dataset)
y=df['Truth']
x=df.drop('Truth',axis=1) # first line of csv has to be labels
#print(y)
#print(x)
#print(x)
#print(y)
scaler = StandardScaler()
scaler.fit(x)
X = scaler.transform(x)
# Splitting the dataset into the Training set and Test set
x_train,x_test,y_train,y_test=train_test_split(X,y,test_size=0.3,random_state=0) # we can play with test_size and random_state
# Fitting Gaussian Naive bayes to the Training set
gnb = GaussianNB()
regressor = gnb.fit(x_train, y_train)
# Predicting the Test set results
y_pred = regressor.predict(x_test)

#for printing accuracy of data
from sklearn import metrics
Y_pred_train = gnb.predict(x_train)
Y_pred_test = gnb.predict(x_test)
train_mse = np.sqrt(metrics.mean_squared_error(y_train, Y_pred_train))
test_mse = np.sqrt(metrics.mean_squared_error(y_test, Y_pred_test))
print("Accuracy for training data: %3.4f. Accuracy for testing data: %3.4f." % (train_mse, test_mse))




# Loading model to compare the results this will be done in our flask server
#model = pickle.load(open('model.pkl','rb'))
#print(model.predict([[answer])) #the answer is the data collected in the answer


#SVM
from sklearn import svm
from sklearn.pipeline import Pipeline
from sklearn.model_selection import GridSearchCV
X_train, X_test, Y_train, Y_test = train_test_split(X, y, test_size=0.3, random_state=1)
#params = {'C':[1,10]}
##check out different values like: Pipeline([('scaler', StandardScaler()), 
##('svm', GridSearchCV(svm.LinearSVC(penalty='l2',loss='hinge', max_iter = 100000), params)), ])
# ('svm', GridSearchCV(svm.LinearSVC(max_iter = 100000), params)), ]) 
##------
params = {'C':[1,10],'kernel':('linear', 'poly', 'rbf', 'sigmoid'),'decision_function_shape': ('ovr', 'ovo')}
svm_pipe_2 = Pipeline([('scaler', StandardScaler()),('svm', GridSearchCV(svm.SVC(),params)), ])

#svm_pipe_2 = Pipeline([('scaler', StandardScaler()),('svm', GridSearchCV(svm.LinearSVC(penalty='l2',loss='hinge', max_iter = 100000), params)), ])
model=svm_pipe_2.fit(X_train, Y_train)

Y_train_pred_1 = svm_pipe_2.predict(X_train)
Y_test_pred_1 = svm_pipe_2.predict(X_test)


print("SVM Train Accuracy: %3.2f" % np.mean(Y_train_pred_1 == Y_train))
print("SVM Test Accuracy: %3.2f" % np.mean(Y_test_pred_1 == Y_test))

# Saving model to disk
pickle.dump(model, open('model.pkl','wb'))

##NN


#from tensorflow.keras.utils import to_categorical
#from keras.models import Sequential
#from keras.layers import Dense
#from tensorflow.keras.optimizers import SGD
#Y = to_categorical(y = y, num_classes = 2)
#X_train, X_test, Y_train, Y_test = train_test_split(X, Y, test_size = 0.2,random_state = 1)

# Create the neural network
#nn = Sequential()
#nn.add(Dense(100, input_shape = (2, ), activation = 'relu'))
#nn.add(Dense(3, activation = 'softmax'))

# Create our optimizer
#sgd = SGD(lr = 0.1)
#Y_train = to_categorical(Y_train,2)
#Y_test = to_categorical(Y_test,2)
# 'Compile' the network to associate it with a loss function,
# an optimizer, and what metrics we want to track
#nn.compile(loss='categorical_crossentropy', optimizer=sgd, 
#          metrics = 'accuracy')
#history = nn.fit(X_train, Y_train, shuffle = True, epochs = 2, validation_data = (X_test, Y_test))


##MLP classifier, got lost here


