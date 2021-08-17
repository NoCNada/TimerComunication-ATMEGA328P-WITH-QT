#include "qform1.h"
#include "ui_qform1.h"

qform1::qform1(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::qform1)
{
    ui->setupUi(this);

    QSerialPort1 = new QSerialPort(this);
    QSerialPort1->setPortName("COM4");
    QSerialPort1->setBaudRate(9600);
    QSerialPort1->open(QSerialPort::ReadWrite);
    connect(QSerialPort1, &QSerialPort::readyRead, this, &qform1::OnQSerialPort1Rx);
}

qform1::~qform1()
{
    delete ui;
    delete QSerialPort1;
}

void qform1::OnQSerialPort1Rx()
{
    int count;
    quint8 *buf;
    QString strhex;
    count=QSerialPort1->bytesAvailable();
    if(count<=0){
        return;
    }
    buf =new quint8[count];
    QSerialPort1->read((char *)buf,count);

    Decodificar(count,buf);
    //strhex = "--> 0x";


   // LeerCabecera(count,buf);
   // mensaje+=strhex;
   // ui->lineEdit_2->setText(mensaje);
    delete[] buf;
    /*
       int count;
       quint8 *buf;
       QString strhex;
       count=QSerialPort1->bytesAvailable();

       if(count<=0){
           return;
       }
       buf =new quint8[count];

       QSerialPort1->read((char *)buf,count);
       //strhex = "--> 0x";

       for(int i=0; i<count; i++)
               strhex = strhex+QString("%1").arg(buf[i], 2, 16, QChar('0')).toUpper();

        LeerCabecera(count,buf);
        mensaje+=strhex;
        ui->lineEdit_2->setText(mensaje);
        delete[] buf;
    */
}

void qform1::Decodificar(int ind, quint8 *buf)
{
    static int caracter=0;
    static uint8_t nBytes=0, cks=0, indLectura=0;
    int indRecepcion=0;

    while (indRecepcion!=ind) {
        switch(caracter){
            case 0:
                if(buf[indRecepcion]=='U'){
                    caracter=1;
                }
            break;
            case 1:
                if(buf[indRecepcion]=='N')
                    caracter=2;
                else {
                    indRecepcion--;
                    caracter=0;
                }
           break;
           case 2:
               if(buf[indRecepcion]=='E')
                   caracter=3;
               else {
                   indRecepcion--;
                   caracter=0;
               }
          break;
          case 3:
            if(buf[indRecepcion]=='R')
                caracter=4;
            else {
                indRecepcion--;
                caracter=0;
            }
          break;
          case 4:
            nBytes=buf[indRecepcion];
            indiceRX=nBytes;
            caracter=5;
          break;
          case 5:
            if(buf[indRecepcion]==0x00)
                caracter=6;
            else {
                indRecepcion--;
                caracter=0;
            }
          break;
          case 6:
            if(buf[indRecepcion]==':'){
                cks= 'U'^'N'^'E'^'R'^nBytes^0x00^':';
                caracter=7;
            }
            else {
                indRecepcion--;
                caracter=0;
            }
          break;
          case 7:
            if(nBytes>1){
                cks^=buf[indRecepcion];
                RX[indLectura++]=buf[indRecepcion]; //guardo datos del buffer en RX para luego ver que llego
            }

            nBytes--;
            if(nBytes==0){
                caracter=0;
                if(cks==buf[indRecepcion]){
                    RecibirComando(indLectura+1-indiceRX);
                }
            }
          break;
        }
        indRecepcion++;
    }
}


void qform1::RecibirComando(uint8_t ind){
    switch(RX[ind++]){
        case 0xF0:
            ui->lineEdit->setText("estoy Vivoo Funciona");
        break;
        case 0xF1:
            ui->lineEdit->setText("subiendo");
        break;
        case 0xF2:
            ui->lineEdit->setText("bajando");
        break;
        case 0xF5:
            ui->lineEdit->setText("ERROR MAX TIEMPO");
        break;
        case 0xF6:
            ui->lineEdit->setText("ERROR MIN TIEMPO");
        break;
    }
}

void qform1::EnviarComando(uint8_t length, uint8_t cmd, uint8_t payload[]){
    uint8_t cks;
    QString strhex;
    QSerialPort1->read((char *)TX, 6 + TX[4]);

    TX[0] = 'U';
    TX[1] = 'N';
    TX[2] = 'E';
    TX[3] = 'R';
    TX[4] = length;
    TX[5] = 0x00;
    TX[6] = ':';
    switch (cmd) {
        case 0xF2: //Enciende o apaga un led
            TX[7] = cmd;
        break;
        case 0xF1: //Indica en que pared rebotó la pelota
            TX[7] = cmd;//CMD
        break;
        case 0xF0: //Alive
            TX[7] = cmd;
        break;
    }


    cks=0;

    for(int i=0; i<TX[4]+6; i++){
        cks ^= TX[i];
    }
    TX[TX[4]+6] = cks;

    if(QSerialPort1->isOpen()){
        QSerialPort1->write((char*)TX, 7 + TX[4]);
    }

}


/*void qform1::EnviarComando(uint8_t length, uint8_t cmd, uint8_t payload[]){
    uint8_t cks;
    static uint8_t indTX=0, testte = 0;
    QString strhex;
    QSerialPort1->read((char *)TX, 6 + TX[indTX-testte]);
    TX[indTX++] = 'U';
    TX[indTX++] = 'N';
    TX[indTX++] = 'E';
    TX[indTX++] = 'R';
    TX[indTX++] = length; //4 pasa a 5
    TX[indTX++] = 0x00; //5 pasa a 6
    TX[indTX++] = ':'; //6 pasa a 7
    testte=3;
    switch (cmd) {
        case 0xF2: //Alive
            TX[indTX++] = cmd;
            testte+=1;
        break;
        case 0xF1: //Envio + para sumar 10ms de "delay" //length = 4 + U N E R '4' 0 : 0XC0 1 2
            TX[indTX++] = 0xF1; //7 pasa a 8
          //  TX[indTX++] = payload[0]; //8 pasa a 9
           // TX[indTX++] = payload[1]; //9 pasa a 10
            testte+=1;
        break;
        case 0xF0: //Alive
            TX[indTX++] = cmd;
            testte+=1;
        break;
        default:
        break;
    }
    cks=0;

    for(int i=0; i<indTX; i++){
        cks ^= TX[i];
    }
    TX[indTX++] = cks; //10 pasa a 11
    testte++;

    if(QSerialPort1->isOpen()){
        QSerialPort1->write((char*)TX, indTX);
    }

    strhex = "--> 0x";

    for(int i=0; i<TX[4]+7; i++){
        strhex = strhex+QString("%1").arg(TX[i], 2, 16, QChar('0')).toUpper() + ' ';
    }
    ui->lineEdit->setText(strhex); */
    /*
    strhex = "--> 0x";

    for(int i=0; i<TX[4]+7; i++){
        strhex = strhex+QString("%1").arg(TX[i], 2, 16, QChar('0')).toUpper();
    }
    ui->lineEdit->setText(strhex); */
//}


void qform1::on_prueba_clicked()
{

}


void qform1::on_SUBIR_clicked()
{
    EnviarComando(0x02,0xF1,payload);
}


void qform1::on_BAJAR_clicked()
{
    EnviarComando(0x02,0xF2,payload);
}


void qform1::on_Alive_clicked()
{
    EnviarComando(0x02,0xF0,payload);
}

