#ifndef QFORM1_H
#define QFORM1_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>

QT_BEGIN_NAMESPACE
namespace Ui { class qform1; }
QT_END_NAMESPACE

class qform1 : public QMainWindow
{
    Q_OBJECT

public:
    qform1(QWidget *parent = nullptr);
    ~qform1();

private slots:
    void on_prueba_clicked();

    void on_SUBIR_clicked();

    void on_BAJAR_clicked();

    void on_Alive_clicked();

private:
    Ui::qform1 *ui;
    QSerialPort *QSerialPort1;

    void OnQSerialPort1Rx();

    void Decodificar(int ind, quint8 *buf);

    void RecibirComando(uint8_t ind);

    void EnviarComando(uint8_t length, uint8_t cmd, uint8_t payload[]);


    uint8_t TX[256], payload[8],RX[256],indiceRX=0,flag11=0,flag22=0;
    QString mensaje="--> 0x";
};
#endif // QFORM1_H
