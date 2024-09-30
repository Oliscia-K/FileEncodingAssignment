/*assignment
 * Prompt for a file and open it (have error message/check)
 * QDataStream to Read (maybe can read without)
 * Read the file (256 inputs)
 * Loop through bytes & make table of counts of each (don't use the normal for (int = 0; i < file.size; ++i) bc the middle section will check the size every time which iterates over full file each time
 * put in a QTableWidget
 * ----------------------
 * Determine Huffman Encoding for the data
 * write a binary file w/ encoding and the encoded file
 * be able to decode output file
 * load, encode, and decode buttons
 * QUESTIONS: ask about sorting based on code
 * */

#include "mainwindow.h"
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent){
    QWidget *center = new QWidget();
    setCentralWidget(center);
    QVBoxLayout *mainLayout = new QVBoxLayout(center);
    QHBoxLayout *buttons = new QHBoxLayout();

    table = new QTableWidget();
    nRows = 256; nCols =4;

    table->setRowCount(nRows);
    table->setColumnCount(nCols);
    table->setHorizontalHeaderLabels(QStringList() << "Code" << "Character" << "Number of instances" << "Encoding");
    table->setColumnHidden(3, true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addLayout(buttons);

    load = new QPushButton("Load File");
    encode = new QPushButton("Encode File");
    decode = new QPushButton("Decode File");
    buttons->addWidget(load);
    buttons->addWidget(encode);
    buttons->addWidget(decode);
    mainLayout->addWidget(table, 1);

    connect(load, &QPushButton::clicked, this, &MainWindow::loadClicked);
    connect(encode, &QPushButton::clicked, this, &MainWindow::encodeClicked);
    connect(decode, &QPushButton::clicked, this, &MainWindow::decodeClicked);

}

MainWindow::~MainWindow() {}

void MainWindow::loadClicked(){
    characters.clear();
    QString fileName = QFileDialog::getOpenFileName();
    QFile file = fileName;
    if (!file.open(QIODevice::ReadOnly)){
        QMessageBox::critical(this, "File Error", QString("There was an error in the process of opening \"%1\". Please try again").arg(fileName), "Okay");
    }else{
        originalFileSize = file.size();
        originalFileInput = file.readAll();
        if (originalFileInput.isEmpty()){
            QMessageBox::critical(this, "Empty File", QString("\"%1\" is an empty file. Please upload a file with information.").arg(fileName), "Okay");
            return;
        }
        table->clearContents();
        int index = 0;
        QByteArray::iterator itByte;
        for (itByte = originalFileInput.begin(); itByte != originalFileInput.end(); ++itByte, ++index){
            if (characters.contains(originalFileInput.at(index))){
                ++characters[originalFileInput[index]];
            }else{
                characters[originalFileInput[index]] = 1;
            }
        }
        QMap<char, int>::iterator itChar;
        int row = 0;
        for (itChar = characters.begin(); itChar != characters.end(); ++itChar){
            if (isprint((unsigned char)itChar.key())){
                table->setRowHidden(row, false);
                QTableWidgetItem *characterCode = new QTableWidgetItem(QString::number((int)itChar.key() + 256, 16).right(2));
                table->setItem(row, 0, characterCode);
                QTableWidgetItem *characterSymbol = new QTableWidgetItem(QString(itChar.key()));
                table->setItem(row, 1, characterSymbol);
                QTableWidgetItem *characterCount = new QTableWidgetItem;
                characterCount->setData(Qt::EditRole, itChar.value());
                table->setItem(row, 2, characterCount);
                ++row;
            }
        }
        while(row < 256){
            table->setRowHidden(row++, true);
        }
        table->setSortingEnabled(true);

    }
}
void MainWindow::encodeClicked(){
    //Huffman needs at least two different characters
    table->setSortingEnabled(false);
    QMultiMap<int, QByteArray> toDo;
    QMap<char, int>::iterator itChar;
    if (characters.size() < 2){
        QMessageBox::critical(this, "Unable to encode", "A Huffman encoding requires more than one type of character", "Okay");
        return;
    }
    table->setColumnHidden(3, false);
    for ( itChar = characters.begin(); itChar != characters.end(); ++itChar){
        QByteArray charByte;
        charByte.append(itChar.key());
        toDo.insert(itChar.value(), charByte);
    }

    QMap<QByteArray, QPair<QByteArray, QByteArray> > children;
    while (toDo.size() > 1){
        QByteArray key1 = toDo.begin().value();
        int w0 = toDo.begin().key();
        toDo.erase(toDo.constBegin());
        QByteArray key2 =  toDo.begin().value();
        int w1 = toDo.begin().key();
        toDo.erase(toDo.constBegin());
        children.insert(key1 + key2, qMakePair(key1, key2));
        toDo.insert(w0+w1, key1+key2);

    }
    int row = 0;
    QVector<QString> encoding(256, 0);
    for (itChar = characters.begin(); itChar != characters.end(); ++itChar){
        QByteArray current = toDo.begin().value();
        QString code;
        QByteArray target(1, itChar.key());
        while (current!=target){
            if (children[current].first.contains(target)){
                code.append("0");
                current = children[current].first;
            }else{
                code.append("1");
                current = children[current].second;
            }
        }
        if(isprint((unsigned char) itChar.key())){
            table->setRowHidden(row, false);
            QTableWidgetItem *characterCode = new QTableWidgetItem(QString::number((int)itChar.key() + 256, 16).right(2));
            table->setItem(row, 0, characterCode);
            QTableWidgetItem *characterSymbol = new QTableWidgetItem(QString(itChar.key()));
            table->setItem(row, 1, characterSymbol);
            QTableWidgetItem *characterCount = new QTableWidgetItem;
            characterCount->setData(Qt::EditRole, itChar.value());
            table->setItem(row, 2, characterCount);
            QTableWidgetItem *characterEncoding = new QTableWidgetItem(code);
            table->setItem(row++, 3, characterEncoding);
        }
        encoding[(unsigned char) itChar.key()] = code;
    }
    while(row < 256){
        table->setRowHidden(row++, true);
    }

    QString encodedFileName = QFileDialog::getSaveFileName(this, "Save");
    QFile encodedFile(encodedFileName);
    QDataStream out(&encodedFile);
    if (!encodedFile.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        QMessageBox::critical(this, "File error", QString("File named \"%1\" could not be opened").arg(encodedFileName), "okay");
        return;
    }

    //Turn original file into encoded version
    QString encodedInput;
    int size = originalFileInput.size();
    for (int index = 0; index < size; ++index){
        encodedInput += encoding[(unsigned char) originalFileInput[index]];
    }
    int encodedInputSize = encodedInput.size();
    QByteArray encodedByteInput;
    for (int index = 0; index < encodedInputSize ; index+=8){
        encodedByteInput.append((char)encodedInput.mid(index, 8).toInt(nullptr, 2));
    }
    out << encoding << encodedInputSize;
    out.writeRawData(encodedByteInput, encodedByteInput.size());
    encodedFile.close();
    QMessageBox::information(this, "File Size Differences", QString("The original import had a size of \"%1\" and the new file has a size of \"%2\"").arg(originalFileSize).arg(encodedFile.size()), "Okay");
    table->setSortingEnabled(true);
}
void MainWindow::decodeClicked(){
    table->setSortingEnabled(false);
    //fill out table and prompt for file
    QString fileName = QFileDialog::getOpenFileName();
    if (fileName.isEmpty()) return;
    QFile file = fileName;
    if (!file.open(QIODevice::ReadOnly)){
        QMessageBox::critical(this, "File Error", QString("There was an error in the process of opening the \"%1\". Please try again").arg(fileName), "Okay");
        return;
    }
    table->clearContents();
    table->setColumnHidden(3, false);
    QDataStream in(&file);
    QVector<QString> codings(256, 0);
    int encodedInputSize;
    in >> codings >> encodedInputSize;
    QByteArray byteInputToDecode((encodedInputSize+7)/8, 0);
    in.readRawData(byteInputToDecode.data(), byteInputToDecode.size());
    QMap<QString, char> encodingKey;
    for (int index = 0; index < 256; ++index){
        if (codings[index] != ""){
            encodingKey.insert(codings[index], (char) index);
        }
    }
    int inputSize = encodedInputSize;
    QString bytesToString = "";
    for (int index = 0; index < encodedInputSize; index+=8){
        int width = encodedInputSize - index;
        if (width > 8){
            width = 8;
        }
        bytesToString += QString::number((unsigned char)byteInputToDecode[index/8], 2).rightJustified(width, '0');
    }
    QString currentCode = "";
    QByteArray final;
    //get counts and actual text
    QMap<char, int> frequencies;
    for (int index = 0; index < inputSize; ++index){
        currentCode += bytesToString[index];
        if (encodingKey.contains(currentCode)){
            final.append(encodingKey[currentCode]);
            if (frequencies.contains(encodingKey[currentCode])){
                ++frequencies[encodingKey[currentCode]];
            }else{
                frequencies[encodingKey[currentCode]] = 1;
            }
            currentCode = "";
        }
    }
    QMap<QString, char>::iterator itEncodingKey;
    int row = 0;
    for (itEncodingKey = encodingKey.begin(); itEncodingKey != encodingKey.end(); ++itEncodingKey){
        if(isprint((unsigned char) itEncodingKey.value())){
            table->setRowHidden(row, false);
            QTableWidgetItem *characterAscii = new QTableWidgetItem(QString::number((int)itEncodingKey.value() + 256, 16).right(2));
            table->setItem(row, 0, characterAscii);
            QTableWidgetItem *characterSymbol = new QTableWidgetItem(QString(itEncodingKey.value()));
            table->setItem(row, 1, characterSymbol);
            QTableWidgetItem *characterCount = new QTableWidgetItem;
            characterCount->setData(Qt::EditRole, frequencies[itEncodingKey.value()]);
            table->setItem(row, 2, characterCount);
            QTableWidgetItem *characterEncoding = new QTableWidgetItem(itEncodingKey.key());
            table->setItem(row++, 3, characterEncoding);
        }
    }
    while(row < 256){
        table->setRowHidden(row++, true);
    }
    QString decodedFileName = QFileDialog::getSaveFileName(this, "Save");
    QFile decodeFile(decodedFileName);
    QDataStream out(&decodeFile);
    if (!decodeFile.open(QIODevice::WriteOnly)){
        QMessageBox::critical(this, "File error", QString("\"%1\" could not be opened").arg(decodeFileName), "okay");
        return;
    }
    out.writeRawData(final.data(), final.size());
    table->setSortingEnabled(true);
}
