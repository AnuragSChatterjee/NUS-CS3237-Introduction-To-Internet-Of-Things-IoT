from flask import Flask,request,render_template
import sqlite3
import pyaudio
import speech_recognition as sr
from nltk.tokenize import word_tokenize
from nltk import pos_tag
from datetime import datetime
import threading
import pyaudio
import wave
import speech_recognition as sr
from nltk.tokenize import word_tokenize
from nltk import pos_tag
import nltk
from datetime import datetime
import time
import pickle

truthList=[]
answerList=[]

firstTime=True
running=False
chunk = 1024  # Record in chunks of 1024 samples
fs = 44100  # Record at 44100 samples per second

frames = []  # Initialize array to store frames
modelPredictions = []
verbs=0
adjectives=0
nouns=0
words=0

p = None
stream = None
x = None
filename = "temp.wav"
# Load the model
model = pickle.load(open('model.pkl','rb'))
app = Flask(__name__)

class SensorData:
    def __init__(self, temperature, humidity, pulse):
        self.temperature = temperature
        self.humidity = humidity
        self.pulse = pulse
    def serialize(self):
        return {
            'temp': self.temperature, 
            'hum': self.humidity,
            'pulse': self.pulse,
        }


#con=sqlite3.connect("deceptiondata.db")
#cursor=con.cursor()
# #use this to create db
#cursor.execute("""CREATE TABLE DECEPTION
#            (AverageTemperatureDif REAL,
#             MaxMinTemperatureDif REAL,
#             AverageHumidityDif REAL,
#             MaxMinHumidityDif REAL,
#             AveragePulseDif REAL,
#             MaxMinPulseDif REAL,
#             WordCount INT,
#             NounCount INT,
#             VerbCount INT,
#             AdjectiveCount INT,
#             Time REAL,
#             Truth INT
#             )""")

def INSERT_data(data):
    con=sqlite3.connect("deceptiondata.db")
    cursor=con.cursor()
    cursor.execute("INSERT INTO DECEPTION VALUES(?,?,?,?,?,?,?,?,?,?,?,?)",data)
    con.commit()
    con.close()

def start_thread():
    global p
    global stream
    global x
    global filename
    global nouns
    p = pyaudio.PyAudio()
    stream = p.open(format=pyaudio.paInt16, channels=2, rate=fs, input=True, frames_per_buffer=chunk)
    x = threading.Thread(target=recording_thread, args=(stream,))
    timestamp = int(datetime.timestamp(datetime.now()))
    filename = str(timestamp) + ".wav" 

    x.start()

def recording_thread(stream):
    global running
    running=True
    global words
    print("\nStart recording\n")
    while(running):
        for i in range(0, int(fs / chunk * 1)):
            data = stream.read(chunk)
            frames.append(data)

    print("\nRecording is finished")

def stop(stream, p):
    stream.stop_stream()
    stream.close()
    p.terminate()

def save(p, filename):
    wf = wave.open(filename, 'wb')
    wf.setnchannels(2)
    wf.setsampwidth(p.get_sample_size(pyaudio.paInt16)) # 16 bits per sample
    wf.setframerate(fs)
    wf.writeframes(b''.join(frames))
    wf.close()

def recognition(filename):
    r = sr.Recognizer()
    audioFile = sr.AudioFile(filename)
    try:
        with audioFile as source:
            r.adjust_for_ambient_noise(source)
            audio = r.record(source)    
        text = r.recognize_google(audio)
        print("You said: " + text)
        process(text)
    except Exception as e: 
        print(e)
        print("Sorry, I could not recognize what you said")

def process(sentence):
    global words
    global verbs
    global adjectives
    global nouns
    global frames
    tokens = word_tokenize(sentence)
    tags = pos_tag(tokens)
    verbs = sum( word[1] == "VBP" or word[1] == "VB" or word[1] == "VBD" or word[1] == "VBG" or word[1] == "VBN" or word[1] == "VBZ" for word in tags )
    adjectives = sum( word[1] == "JJ" or word[1] == "JJS" or word[1] == "JJR" for word in tags )
    nouns = sum( word[1] == "NN" or word[1] == "NNS" or word[1] == "NNP" or word[1] == "NNPS" for word in tags )
    words = len(tokens)
#get the prediction for all answers on this site
@app.route('/homePage')
def returnHtml():
    return render_template('homePage.html',modelPredictions=modelPredictions)

@app.route('/collectTruthData', methods=['GET', 'POST'])
def postTruthSensorData():
    content = request.get_json() #get data from json
    global truthList
    truthList.append(SensorData(content["temperature"],content["humidity"],content["pulse"]))
    return "Truth data collection in progress."

@app.route('/collectAnswerData', methods=['GET', 'POST'])
def postSomething():
    global firstTime
    global answerList
    global startTime
    if(firstTime):
        startTime=time.time()
        start_thread()
        firstTime=False
    content = request.get_json() #get data from json
    
    answerList.append(SensorData(content["temperature"],content["humidity"],content["pulse"])) 
    return "Answer collection in progress."

#get the differences compared to the values when the answer was true
@app.route('/getTruth', methods=['GET', 'POST']) 
def getIsTruth():
    global answerList
    global truthList
    global words
    global verbs
    global adjectives
    global nouns
    global frames
    global stream
    global p
    global filename
    global x
    global running
    global firstTime
    global startTime
    global endTime
    
    answerLength=len(answerList)
    truthLength=len(truthList)
    running = False
    x.join()
    running = True
    firstTime = True
    stop(stream, p)
    save(p, filename)
    recognition(filename)
    endTime= time.time()
    #insert into db
    if(answerLength>1):
        elapsedTime=endTime-startTime
        avgTempDif=(sum(c.temperature for c in answerList)/answerLength)-(sum(c.temperature for c in truthList)/truthLength)
        maxMinTempDif=max(c.temperature for c in answerList)-min(c.temperature for c in answerList)-(max(c.temperature for c in truthList)-min(c.temperature for c in truthList))
        avgHumDif=(sum(c.humidity for c in answerList)/answerLength)-(sum(c.humidity for c in truthList)/truthLength)
        maxMinHumDif=max(c.humidity for c in answerList)-min(c.humidity for c in answerList)-(max(c.humidity for c in truthList)-min(c.humidity for c in truthList))
        avgPulseDif=(sum(c.pulse for c in answerList)/answerLength)-(sum(c.pulse for c in truthList)/truthLength)
        maxMinPulseDif=max(c.pulse for c in answerList)-min(c.pulse for c in answerList)-(max(c.pulse for c in truthList)-min(c.pulse for c in truthList))
        
        
        prediction = model.predict([[avgTempDif,maxMinTempDif,avgHumDif,maxMinHumDif,avgPulseDif,maxMinPulseDif,words,nouns,verbs,adjectives,elapsedTime]])

        if(prediction==0):
            modelPredictions.append("true")
            data=[avgTempDif,maxMinTempDif,avgHumDif,maxMinHumDif,avgPulseDif,maxMinPulseDif,words,nouns,verbs,adjectives,elapsedTime,0]
        else:
            modelPredictions.append("false")
            data=[avgTempDif,maxMinTempDif,avgHumDif,maxMinHumDif,avgPulseDif,maxMinPulseDif,words,nouns,verbs,adjectives,elapsedTime,1]
        INSERT_data(data)  
        frames.clear()
    answerList.clear()
    

    return "Prediction done"

def main():
    app.run(host='0.0.0.0',port=5000,debug=True)
    
if __name__ == '__main__':
    main()


    


#@app.route('/answer', methods = ['POST'])
#def evaulate_answer():
    #answer = request.get_json()

    # Make prediction 
    #prediction = model.predict([[np.array(data['exp'])]])

    #return jsonify(prediction)
 
