#include <iostream>
#include <ios>
#include <fstream>
#include <chrono>
#include <thread>
#include <filesystem>

typedef struct WAV_HEADER{
    char                RIFF[4];        // RIFF Header      Magic header
    uint32_t       ChunkSize;      // RIFF Chunk Size  
    char                WAVE[4];        // WAVE Header      
    char                fmt[4];         // FMT header       
    uint32_t       Subchunk1Size;  // Size of the fmt chunk                                
    unsigned short      AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM 
    unsigned short      NumOfChan;      // Number of channels 1=mono 2=stereo                   
    uint32_t       SamplesPerSec;  // Sampling Frequency in Hz                             
    uint32_t       bytesPerSec;    // bytes per second 
    unsigned short      blockAlign;     // 2=16-bit mono, 4=16-bit stereo 
    unsigned short      bitsPerSample;  // Number of bits per sample      
    char                Subchunk2ID[4]; // "data"  string   
    uint32_t       Subchunk2Size;  // Sampled data length    
}wav_hdr;

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

uint16_t bytesToShort(char first, char second){
    return (uint16_t)((unsigned char)first | (unsigned char)second << 8);
}

uint32_t bytesToInt32(char first, char second, char third, char fourth){
    return (uint32_t)((unsigned char)first | 
        (unsigned char)second << 8 |
        (unsigned char)third << 16 | 
        (unsigned char)fourth << 24); 
}

void readWaveData(char* array, size_t size){
    uint32_t temp = (uint32_t)(array); 
    temp += sizeof(char)*(size-1);
    for(; array<(char*)temp; array+=2){
    // Every sample is a point on a wave
    // So if the point is really high it's going to be louder
    unsigned short waveSample = (unsigned short)((unsigned char)*array | (unsigned char)*(array+1) << 8); 
    waveSample*=1;
    memcpy(array, &waveSample, 2);
    }
}

void writeBackToFile(char* header, char* buffer, size_t sizeOfBuffer){
    std::string filePath = (std::filesystem::current_path() / "output.wav").string();
    std::fstream ofs(filePath.c_str(), std::ios::out | std::ios::binary);
    if (!ofs){
        std::cerr << filePath.c_str() << " Not found" << std::endl;
    }
    
    ofs.write(header, 44);
    ofs.write(buffer, sizeOfBuffer);
    
}

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

int main(){

    std::string filePath = (std::filesystem::current_path() / "file.wav").string();
    std::fstream ifs(filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
    if (!ifs){
        std::cerr << filePath.c_str() << " Not found " << std::endl;
    }

    long length = static_cast<long>(ifs.tellg())-44;
    long lengthreal = ifs.tellg();
    //std::cout << length << std::endl;
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
    
    writeBackToFileCsv(reinterpret_cast<unsigned char*>(buffer), length, "out1.csv");
    readWaveData(buffer, length);
    writeBackToFile(header, buffer, length);
    writeBackToFileCsv(reinterpret_cast<unsigned char*>(buffer), length, "out2.csv");
    printAttri(wavHeader);

    delete buffer;
    delete header;
    return 0;
}