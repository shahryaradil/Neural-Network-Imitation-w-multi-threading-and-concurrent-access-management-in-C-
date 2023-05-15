#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <fstream>
using namespace std;

int row1 = 0;
int col1 = 0;
int row2 = 0;
int col2 = 0;
int count = 0;
pthread_mutex_t mutex;

vector<double> input;
vector<vector<double>> inputLayer;
vector<vector<vector<double>>> hidden_layers;
vector<vector<double>> outputLayerWeights;

int NUM_HIDDEN_LAYERS = 0;
int NUM_INPUT_NEURONS = 0;
int NUM_OUTPUT_NEURONS = 0;
int NUM_HIDDEN_NEURONS = 0;

vector<double> output;
double output_reader[2];
vector<double> prev_arr;
vector<double> reader;


void readfile(){
    ifstream inputFile("/home/fouad/Desktop/Configuration.txt");
    string line;

    int layerNum = 0;
    int rowNum = 0;

    while (getline(inputFile, line)) {
        double weight;
        int colNum = 0;
        int k = 0;
        string w = "";
        

        if (line == "Input layer weights") {
            
            layerNum = 0;
            rowNum = 0;

        }
        else if (line.find("Output layer weights") != string::npos) {
            layerNum = NUM_HIDDEN_LAYERS + 1;
            rowNum = 0;

        }
        else if(line.find("Input data") != string::npos){
            layerNum = 20;
            rowNum = 0;
        }
        else {
            for (int i = 0; i < 10; i++) {
                string a = "Hidden layer " + to_string(i) + " weights";

                if (line.find(a) != string::npos) {

                    layerNum++;
                    rowNum = 0;

                }
                // else{
                //     break;
                // }
            }
        }


       

        if (line[k] == ' ' || line[k] == '-' || isdigit(line[k]) && !line.empty()) {
           

            while (line[k] != '\0') {
                while (line[k] != '\0') {
                    if (line[k] == ' ')
                        k++;
                    if (line[k] == '-' || line[k] == '.') {
                        // Handle negative numbers or decimal points if needed
                    }
                    else if (line[k] == ',') {
                        k++;
                        break;
                    }

                    if (line[k] != ' ' && (line[k] == '.' || line[k] == '-' || isdigit(line[k])))
                        w += line[k];
                    k++;
                }

                
                weight = stod(w);


                if (layerNum == 0) {
                    inputLayer[rowNum-1][colNum] = weight;

                }
                else if (layerNum > 0 && layerNum < NUM_HIDDEN_LAYERS + 1) {
                    
                    hidden_layers[layerNum -1][rowNum-1][colNum] = weight;
                   
                }
                else if (layerNum == NUM_HIDDEN_LAYERS + 1) {
                    
                    outputLayerWeights[rowNum-1][colNum] = weight;
                }
                else if (layerNum == 20) {
                    
                    input[colNum] = weight;
                }

                colNum++;
                w = "";

            }
        }
      
        rowNum++;
             
    }
    inputFile.close();
}

void* init_mul(void* args){
    pthread_mutex_lock(&mutex);
    double sum = 0;
    for(int i=0; i < col1; i++){
        sum += input[i] * inputLayer[i][count];
    }
    output[count] = sum;
    count++;
    pthread_mutex_unlock(&mutex);
    return NULL;
}
void* main_mul(void* args){
    pthread_mutex_lock(&mutex);
    int index = *((int*)args);
    double sum = 0;

    for(int i=0; i < col1; i++){
        sum += prev_arr[i] * hidden_layers[index][i][count];
    }
    output[count] = sum;
    count++;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char* argv[]) {
    setuid(0);
    string pipename;
    if (argc >= 2){
        if (strcmp(argv[1], argv[2]) == 0){
            row1 = 1;
            col1 = 2;
            row2 = col1;
            col2 = stoi(argv[3]);

            input.resize(col1);
            inputLayer.resize(row2);
            for(int i=0; i< row2; i++) {
                inputLayer[i].resize(col2);
            }

            row1 = 1;
            col1 = stoi(argv[3]);
            row2 = col1;
            col2 = row2;

            hidden_layers.resize(stoi(argv[2])-2);
            for(int i=0; i<(stoi(argv[2])-2); i++){
                hidden_layers[i].resize(row2);
                for(int j=0; j<row2; j++){
                    hidden_layers[i][j].resize(col2);
                }
            }

            row1 = 1;
            col1 = stoi(argv[3]);
            row2 = col1;
            col2 = 1;

            outputLayerWeights.resize(row2);
            for(int i=0; i < row2; i++){
                outputLayerWeights[i].resize(col2);
            }

            NUM_HIDDEN_LAYERS = stoi(argv[2]) - 2;
            NUM_INPUT_NEURONS = 2;
            NUM_OUTPUT_NEURONS = 1;
            NUM_HIDDEN_NEURONS = stoi(argv[3]);


            readfile();

            string s_pipe_num = argv[1];
            int prev_s_pipe_num = stoi(s_pipe_num) - 1;
            pipename = "pipe" + s_pipe_num;
            string prev_pipename = "pipe" + to_string(prev_s_pipe_num);
            int num_of_pipes = stoi(s_pipe_num);

            num_of_pipes++;
            s_pipe_num = to_string(num_of_pipes);
            prev_arr.resize(col1);
            int fd = open(prev_pipename.c_str(), O_RDONLY);
            if (fd == -1){
                cout<<"Error opening"<<endl;
            }
            read(fd, &(prev_arr.front()), sizeof(double) * col1);
            close(fd);

            // cout<<"Prev Arr : "<<endl;
            // for(int i=0; i<col1; i++){
            //     cout<<prev_arr[i]<<" ";
            // }
            // cout<<endl;
            //  cout<<"Out Arr : "<<endl;
            // for(int i=0; i<col1; i++){
            //     cout<<outputLayerWeights[i][0]<<" ";
            // }
            // cout<<endl;
            double output_sum = 0;
            for(int i=0; i < col1;i++){
                cout<<prev_arr[i] <<" * "<<outputLayerWeights[i][0]<<" = "<<prev_arr[i] * outputLayerWeights[i][0]<<" ";
                //double clear = 0.0000001 * 0.0000001;
                output_sum += prev_arr[i] * outputLayerWeights[i][0];
                //sleep(1);
            }
            cout<<endl;
            cout<<"Final : "<<output_sum<<endl;
            output_reader[0] = ((output_sum*output_sum)*(output_sum)+1)/2;
            output_reader[1] = ((output_sum)+1)/2;
            fd = open(prev_pipename.c_str(), O_WRONLY);
            write(fd, &output_reader, sizeof(double) * 2);
            close(fd);
        }
        else{
            row1 = 1;
            col1 = stoi(argv[3]);
            row2 = col1;
            col2 = 1;

            outputLayerWeights.resize(row2);
            for(int i=0; i < row2; i++){
                outputLayerWeights[i].resize(col2);
            }

            row1 = 1;
            col1 = 2;
            row2 = col1;
            col2 = stoi(argv[3]);

            input.resize(col1);
            inputLayer.resize(row2);
            for(int i=0; i< row2; i++) {
                inputLayer[i].resize(col2);
            }

            row1 = 1;
            col1 = stoi(argv[3]);
            row2 = col1;
            col2 = row2;
        
            hidden_layers.resize(stoi(argv[2])-2);
            for(int i=0; i<(stoi(argv[2])-2); i++){
                hidden_layers[i].resize(row2);
                for(int j=0; j<row2; j++){
                    hidden_layers[i][j].resize(col2);
                }
            }

            NUM_HIDDEN_LAYERS = stoi(argv[2]) - 2;
            NUM_INPUT_NEURONS = 2;
            NUM_OUTPUT_NEURONS = 1;
            NUM_HIDDEN_NEURONS = stoi(argv[3]);

            readfile();

            string s_pipe_num = argv[1];
            int prev_s_pipe_num = stoi(s_pipe_num) - 1;
            int three_d_index = stoi(s_pipe_num) - 2;
            pipename = "pipe" + s_pipe_num;
            string prev_pipename = "pipe" + to_string(prev_s_pipe_num);
            int num_of_pipes = stoi(s_pipe_num);
            mkfifo(pipename.c_str(), 666);
            num_of_pipes++;
            s_pipe_num = to_string(num_of_pipes);
            prev_arr.resize(col2);
            int fd = open(prev_pipename.c_str(), O_RDONLY);
            if (fd == -1){
                cout<<"Error opening"<<endl;
            }
            read(fd, &(prev_arr.front()), sizeof(double) * col2);
            close(fd);
            pid_t pid = fork();
            if (pid == 0){
                vector<pthread_t> thread_id(col2);
                output.resize(col2);
                for (int i=0; i<col2; i++){
                    pthread_create(&thread_id[i], NULL, main_mul, (void*)&three_d_index);
                }
                while(count != col2);
                count = 0;
                fd = open(pipename.c_str(), O_WRONLY);  
                write(fd, &(output.front()), sizeof(double)*col2);
                close(fd);
                //execl("a.out", "sudo", "./a.out", s_pipe_num.c_str(), argv[3], NULL);
                execl("/usr/bin/sudo", "sudo", "./a.out", s_pipe_num.c_str(), argv[2], argv[3], NULL);

            }
            else{
                int fd = open(pipename.c_str(), O_RDONLY);
                reader.clear();
                reader.resize(col2); 
                if (fd == -1){
                    cout<<"Error opening f"<<endl; 
                }
                read(fd, &(reader.front()), sizeof(double)*col2);
                close(fd);
                for (int i=0; i<col2; i++){
                    cout<<reader[i]<<endl;
                }
                cout<<endl;
                fd = open(pipename.c_str(), O_WRONLY);
                write(fd, &(reader.front()), sizeof(double)*col2);
                close(fd);

                //Back
                //string back_pipe = "pipe" + to_string(num_of_pipes - 1);
                fd = open(pipename.c_str(), O_RDONLY);
                read(fd, &output_reader, sizeof(double)*2);
                close(fd);
                
                cout<<"output read : "<<output_reader[0]<<endl;
                fd = open(prev_pipename.c_str(), O_WRONLY);
                write(fd, &output_reader, sizeof(double)*2);
                close(fd);
                //cout<<argv[1]<<endl;
                //wait(NULL);
            }
        }
    }
    else {
        int layers, neurons, runs, run_c = 0;
        cout<<"Enter number of cycles to run: ";
        cin>>runs;
        cout<<"Enter number of layers: "; 
        cin >> layers;
        cout<<"Enter number of neurons: ";
        cin >> neurons;
        //layers++;

        row1 = 1;
        col1 = neurons;
        row2 = col1;
        col2 = 1;

        outputLayerWeights.resize(row2);
        for(int i=0; i < row2; i++){
            outputLayerWeights[i].resize(col2);
        }

        row1 = 1;
        col1 = neurons;
        row2 = col1;
        col2 = row2;

        hidden_layers.resize(layers-2);
        for(int i=0; i<(layers-2); i++){
            hidden_layers[i].resize(row2);
            for(int j=0; j<row2; j++){
                hidden_layers[i][j].resize(col2);
            }
        }

        row1 = 1;
        col1 = 2;
        row2 = col1;
        col2 = neurons;

        input.resize(col1);
        inputLayer.resize(row2);
        for(int i=0; i< row2; i++) {
            inputLayer[i].resize(col2);
        }

        NUM_HIDDEN_LAYERS = layers - 2;
        NUM_INPUT_NEURONS = 2;
        NUM_OUTPUT_NEURONS = 1;
        NUM_HIDDEN_NEURONS = neurons;


        readfile();

        while(run_c < runs){
            pipename = "pipe1";
            count = 0;
            mkfifo(pipename.c_str(), 666);
            string s_layers = to_string(layers);
            string s_neurons = to_string(neurons);
            cout<<"Input Vals : "<<input[0]<<" , "<<input[1]<<endl;
            pid_t pid = fork();
            if (pid == 0){
                vector<pthread_t> thread_id;
                thread_id.resize(neurons);
                output.resize(col2);
                for (int i=0; i<neurons; i++){
                    pthread_create(&thread_id[i], NULL, init_mul, NULL);
                }
                while(count != col2);
                count = 0;
                int fd = open(pipename.c_str(), O_WRONLY);  
                write(fd, &(output.front()), sizeof(double)*col2);
                close(fd);
                
                //execl("a.out", "sudo", "./a.out", "2", s_layers.c_str(), NULL);
                execl("/usr/bin/sudo", "sudo", "./a.out", "2", s_layers.c_str(), s_neurons.c_str(),NULL);

            }
            else{
                reader.resize(col2); 
                int fd = open(pipename.c_str(), O_RDONLY);
                read(fd, &(reader.front()), sizeof(double)*col2);
                close(fd);
                for (int i=0; i<neurons; i++){
                    cout<<reader[i]<<endl;
                }
                cout<<endl;
                fd = open(pipename.c_str(), O_WRONLY);
                write(fd, &(reader.front()), sizeof(double)*col2);
                close(fd);

                //Back
                fd = open(pipename.c_str(), O_RDONLY);
                read(fd, &output_reader, sizeof(double)*2);
                close(fd);
        
                cout<<"Value after back = "<<output_reader[0]<<" , "<<output_reader[1]<<endl;
                input[0] = output_reader[0];
                input[1] = output_reader[1];
                //cout<<"1"<<endl;
                //wait(NULL);
                sleep(2);
                for(int i=1; i<= layers; i++){
                    string rem_pipe = "pipe" + to_string(i);
                    unlink(rem_pipe.c_str());
                }
            }
            run_c++;
        }
    }

    if(argc >= 2){
    
    cout<<"Returning : " << argv[1] <<endl;
    }
    else{
        cout<<"Returning : 1"<<endl;
    }
    return 0;
}
