#include <cstdio>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "read_data.h" 

// TODO There's a problem with wav files that have 8 bit sample sizes, it has to do with how the program parses them and with the function pointer

void writeBackToFileCsv(unsigned char* buffer, size_t sizeOfBuffer, std::string fileName){
    std::string filePath = (std::filesystem::current_path() / fileName).string();
    std::fstream ofs(filePath.c_str(), std::ios::out | std::ios::binary);
    if (!ofs){
        std::cerr << filePath.c_str() << " Not found" << std::endl;
    }
    for(int i = 0; i<sizeOfBuffer-1; i+=2){
        ofs << ((unsigned short)(buffer[i]) | (unsigned short)(buffer[i+1]) << 8) << ", ";
    }
}
void writeBackToFile(char* header, char* buffer, size_t sizeOfBuffer, std::string filename){
    std::string filePath = (std::filesystem::current_path() / filename).string();
    std::fstream ofs(filePath.c_str(), std::ios::out | std::ios::binary);
    if (!ofs){
        std::cerr << filePath.c_str() << " Not found" << std::endl;
    }
    
    ofs.write(header, 44);
    ofs.write(buffer, sizeOfBuffer);
}
void printAttri(wav_hdr wavHeader){
    using namespace std;
     cout << "RIFF header                :" << wavHeader.RIFF[0] 
                                                << wavHeader.RIFF[1] 
                                                << wavHeader.RIFF[2] 
                                                << wavHeader.RIFF[3] << endl;

        cout << "WAVE header                :" << wavHeader.WAVE[0] 
                                                << wavHeader.WAVE[1] 
                                                << wavHeader.WAVE[2] 
                                                << wavHeader.WAVE[3] 
                                                << endl;

        cout << "FMT                        :" << wavHeader.fmt[0] 
                                                << wavHeader.fmt[1] 
                                                << wavHeader.fmt[2] 
                                                << wavHeader.fmt[3] 
                                                << endl;

        cout << "Data size                  :" << wavHeader.ChunkSize << endl;

        // Display the sampling Rate form the header
        cout << "Sampling Rate              :" << wavHeader.SamplesPerSec << endl;
        cout << "Number of bits used        :" << wavHeader.bitsPerSample << endl;
        cout << "Number of channels         :" << wavHeader.NumOfChan << endl;
        cout << "Number of bytes per second :" << wavHeader.bytesPerSec << endl;
        cout << "Data length                :" << wavHeader.Subchunk2Size << endl;
        cout << "Audio Format               :" << wavHeader.AudioFormat << endl;
        // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM 

        cout << "Block align                :" << wavHeader.blockAlign << endl;

        cout << "Data string                :" << wavHeader.Subchunk2ID[0] 
                                                << wavHeader.Subchunk2ID[1]
                                                << wavHeader.Subchunk2ID[2] 
                                                << wavHeader.Subchunk2ID[3] 
                                                << endl;
}
uint8_t bytesToByte(char first){
    return (uint8_t)((unsigned char) first);
}
uint16_t bytesToShort(char first, char second){
    return (uint16_t)((unsigned char)first | (unsigned char)second << 8);
}
uint32_t bytesToInt32(char first, char second, char third, char fourth){
    return (uint32_t)((unsigned char)first | 
        (unsigned char)second << 8 |
        (unsigned char)third << 16 | 
        (unsigned char)fourth << 24); 
}
template <typename sampleSize>
void readWaveData(char* array, size_t size){
    sampleSize waveSample;
    const int step = sizeof(waveSample);
    uint32_t temp = (uint32_t)(array); 
    temp += sizeof(char)*(size);
    uint16_t (*conversionType)(char, char, char, char);
    std::cout << step << std::endl;
    // okay this switch isn't working for 8 bit sample values and i'm unsure if it's the struct but try a struct sort of like this from stackoverflow:
    // okay i just found out its not the switch but is actually a problem with that code but i still think a struct would be cleaner
    /*
        typedef struct {
          char *name;      // suggest const char *name
          void (*func)();  // no parameter info nor checking
        } command_info;

        char buf[100];
        // void setbuf(FILE * restrict stream, char * restrict buf);
        command_info fred = { "my_setbuf", setbuf };

        // Both compile, 2nd is UB.
        fred.func(stdin, buf);  // No parameter checking.
    */
    std::cout << size << std::endl;
    switch(step){
        case 2:{
        conversionType = (uint16_t (*)(char, char, char, char)) bytesToShort;
        break;}
        case 4:{
        uint32_t (*conversionType)(char, char, char, char) = (uint32_t (*)(char, char, char, char)) bytesToInt32;
        break;}
    }
    
    for(; array<(char*)temp; array+=step){
    // Every sample is a point on a wave
    // So if the point is really high it's going to be louder
    // (step!=1) ? (*conversionType)(*array, *(array+1), *(array+2), *(array+3)) : 
    waveSample = (*conversionType)(*array, *(array+1), *(array+2), *(array+3));
    //waveSample = (*conversionType)(*array); // the problem was that it was going out of bounds as I forgot to change it to be a set length
    waveSample*=1;
    memcpy(array, &waveSample, step);
    }
}
// Specialization for 24 bit samples
template<>
void readWaveData<int24>(char* array, size_t size){
    uint32_t temp = (uint32_t)(array); 
    temp += sizeof(char)*(size-2); // <- remember to check this because it should be different
    const int step = 24/8;
    int24 waveSample;
    for(; array<(char*)temp; array+=step){
    waveSample.data = (uint32_t)((unsigned char)*array | (unsigned char)*(array+1) << 8 | (unsigned char)*(array+2)<<16); 
    waveSample.data*=1;
    memcpy(array, &waveSample, step);
    }
}
const std::string filenamein = "file.wav";
int main(){
    std::string filePath = (std::filesystem::current_path() / filenamein).string();
    std::fstream ifs(filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
    if (!ifs){
        std::cerr << filePath.c_str() << " Not found " << std::endl;
    }

    long length = static_cast<long>(ifs.tellg())-44;
    char* buffer = new char[length];
    ifs.seekg(0, std::ios::beg);
    char* header = new char[44];
    ifs.read(header, 44);
    ifs.read(buffer, length);
    buffer[length]='\0';

    
    wav_hdr wavHeader;

    memcpy(wavHeader.RIFF, &header[0], 4);
    
    wavHeader.ChunkSize = bytesToInt32(header[4], header[5], header[6], header[7]);
    
    memcpy(wavHeader.WAVE, &header[8], 4);

    memcpy(wavHeader.fmt, &header[12], 4);

    wavHeader.Subchunk1Size = bytesToInt32(header[16], header[17], header[18], header[19]);

    wavHeader.AudioFormat = bytesToShort(header[20], header[21]);

    wavHeader.NumOfChan = bytesToShort(header[22], header[23]);

    wavHeader.SamplesPerSec = bytesToInt32(header[24], header[25], header[26], header[27]);

    wavHeader.bytesPerSec = bytesToInt32(header[28], header[29], header[30], header[31]);

    wavHeader.blockAlign = bytesToShort(header[32], header[33]);

    wavHeader.bitsPerSample = bytesToShort(header[34], header[35]);

    memcpy(wavHeader.Subchunk2ID, &header[36], 4);

    wavHeader.Subchunk2Size = bytesToInt32(header[40], header[41], header[42], header[43]);
    printAttri(wavHeader); // with files with 8 bit sizes for their samples this throws an error ?
    std::cout << wavHeader.WAVE[0] << wavHeader.WAVE[1];
    std::cout << std::endl;
    //writeBackToFileCsv((unsigned char*)buffer, length, "out81.csv");
    std::cout << std::endl;
    switch(wavHeader.bitsPerSample){
        case 8:
        readWaveData<unsigned char>(buffer, length);
        case 16:
        readWaveData<unsigned short>(buffer, length);
        case 24:
        readWaveData<int24>(buffer, length);
        case 32:
        readWaveData<unsigned short>(buffer, length);
    }
    //writeBackToFileCsv(reinterpret_cast<unsigned char*>(buffer), length, "out82.csv");
    writeBackToFile(header, buffer, length, "output4.wav");
    delete header;
    delete buffer;
    return 0;
}