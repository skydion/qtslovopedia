#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    thread = NULL;
    actionExit = actionScan = NULL;

    font.setFamily("Tahoma");
    font.setPointSize(14);

    ui->textEditQuestion->setFont(font);
    ui->plainTextEditText->setFont(font);
    ui->lineEditWord->setFont(font);
    ui->comboBoxVocabulary->setFont(font);

    //    ui->pushButtonNext->setFont(font);
    //    ui->pushButtonPrevious->setFont(font);
    //    ui->pushButtonStart->setFont(font);
    //    ui->pushButtonStop->setFont(font);

    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("slovopedia");
    db.setUserName("taras");
    db.setPassword("su112per");
    db.open();

    // File menu
    //    actionExit = new QAction(QIcon(":/icons/x16.png"), QString(tr("Exit")), this);
    //    connect(actionExit, SIGNAL(triggered()), this, SLOT(close()));
    //
    //    actionScan = new QAction(QIcon(":/icons/search16.png"), QString(tr("Scan")), this);
    //    connect(actionScan, SIGNAL(triggered()), this, SLOT(scanVocabulary()));
    //    ui->mainToolBar->addAction(actionScan);
    //
    //    QMenu *menuFile = new QMenu(tr("File"), this);
    //    ui->menuBar->addMenu(menuFile);
    //    menuFile->addAction(actionScan);
    //    menuFile->addAction(actionExit);
    ui->mainToolBar->setVisible(false);

    vocabulary[29] = "УСЕ (Універсальний словник-енциклопедія)";
    vocabulary[35] = "Орфографічний словник української мови";
    vocabulary[49] = "Фразеологічний словник української мови";
    vocabulary[31] = "Словник синонімів Полюги";
    vocabulary[41] = "Словник синонімів Караванського";
    vocabulary[36] = "Словник іншомовних слів";
    vocabulary[42] = "Словник іншомовних слів Мельничука";
    vocabulary[46] = "Словник англіцизмів";
    vocabulary[38] = "Eкономічна енциклопедія";
    vocabulary[27] = "Словник мови Стуса";
    vocabulary[39] = "Словник іншомовних соціокультурних термінів";
    vocabulary[40] = "Енциклопедія політичної думки";
    vocabulary[43] = "Словник церковно-обрядової термінології";
    vocabulary[44] = "Архітектура і монументальне мистецтво";
    vocabulary[45] = "Словник-антисуржик";
    vocabulary[48] = "Словник термінів, уживаних у чинному Законодавстві України";
    vocabulary[50] = "Словник бюджетної термінології";
    vocabulary[51] = "Термінологічний словник з економіки праці";
    vocabulary[52] = "Глосарій термінів Фондового ринку";
    vocabulary[53] = "Моделювання економіки";
    vocabulary[54] = "Власні імена людей";
    vocabulary[57] = "Словар українського сленгу";
    vocabulary[58] = "Музичні терміни";
    vocabulary[59] = "Тлумачний словник з інформатики та інформаційних систем для економістів";
    vocabulary[61] = "Управління якістю";

    int idx = 0;
    QMap<int, QString>::iterator i;
    for (i = vocabulary.begin(); i != vocabulary.end(); ++i)
    {
        ui->comboBoxVocabulary->insertItem(idx, i.value(), QVariant(i.key()));
        idx++;
    }

    mapper = NULL;
    mapper = new QDataWidgetMapper(this);
    if (mapper)
    {
        model = NULL;
        model = new QSqlQueryModel(this);

        if (model)
            setupModel();

        mapper->addMapping(ui->lineEditWord, 0);
        mapper->addMapping(ui->textEditQuestion, 1);
        mapper->addMapping(ui->plainTextEditText, 2);
        mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
        //	mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

        connect(ui->pushButtonPrevious, SIGNAL(clicked()), mapper, SLOT(toPrevious()));
        connect(ui->pushButtonNext, SIGNAL(clicked()), mapper, SLOT(toNext()));
        //	connect(ui->pushButtonSubmit, SIGNAL(clicked()), mapper, SLOT(submit()));
        connect(ui->pushButtonSubmit, SIGNAL(clicked()), this, SLOT(submitButton()));
        connect(mapper, SIGNAL(currentIndexChanged(int)), this, SLOT(updateButtons(int)));

        mapper->toFirst();
    }

    connect(ui->comboBoxVocabulary, SIGNAL(currentIndexChanged(int)), this, SLOT(setupModel()));

    ui->pushButtonStop->setDisabled(true);
    connect(ui->pushButtonStart, SIGNAL(toggled(bool)), this, SLOT(scanVocabulary(bool)));
    connect(ui->pushButtonStop, SIGNAL(toggled(bool)), this, SLOT(stopScan(bool)));

    connect(ui->pushButtonExport, SIGNAL(clicked()), this, SLOT(exportVocabulary()));
}

MainWindow::~MainWindow()
{
    delete ui;

    db.close();
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);

    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;

    default:
        break;
    }
}

bool MainWindow::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Close:
        stopScan(true);
        break;

    default:
        break;
    }

    return QMainWindow::event(e);

}

void MainWindow::scanVocabulary(bool toggled)
{
    if (toggled)
    {
        thread = new ScanThread(this);

        if (thread)
        {
            connect(thread, SIGNAL(parsedFile(QString)), this, SLOT(updateStatusBar(QString)));
            connect(thread, SIGNAL(finished()), this, SLOT(scanFinished()));

            selectVocabulary(ui->comboBoxVocabulary->currentIndex());

            thread->setDB(&db);
            thread->startScanning();
            thread->start(QThread::NormalPriority);

            ui->comboBoxVocabulary->setDisabled(true);
            ui->pushButtonStart->setDisabled(true);
            ui->pushButtonStop->setDisabled(false);
        }
    }
}

void MainWindow::setupModel(void)
{
    QString q("SELECT _word, _question, _text FROM slovopedia.works WHERE _vocabulary = %1;");
    int index = ui->comboBoxVocabulary->currentIndex();
    q = q.arg(ui->comboBoxVocabulary->itemData(index, Qt::UserRole).toInt());

    model->setQuery(q);

    mapper->setModel(model);
    mapper->toFirst();

    vocabularyId = ui->comboBoxVocabulary->itemData(index, Qt::UserRole).toInt();
}

void MainWindow::updateButtons(int row)
{
    ui->pushButtonPrevious->setEnabled(row > 0);
    ui->pushButtonNext->setEnabled(row < model->rowCount() - 1);
}

void MainWindow::stopScan(bool toggled)
{
    if (toggled)
    {
        if (thread)
        {
            updateStatusBar("");
            ui->pushButtonStop->setDisabled(true);
            thread->stopScanning();

            if (thread->wait())
            {
                ui->comboBoxVocabulary->setDisabled(false);
                ui->pushButtonStart->setDisabled(false);
                ui->pushButtonStart->setChecked(false);

                ui->pushButtonStop->setChecked(false);

                thread->disconnect();

                delete thread;
                thread = NULL;
            }
        }
    }
}

void MainWindow::scanFinished(void)
{
    updateStatusBar("");
    ui->pushButtonStop->setDisabled(true);

    if (thread)
        thread->stopScanning();

    ui->comboBoxVocabulary->setDisabled(false);
    ui->pushButtonStart->setDisabled(false);
    ui->pushButtonStart->setChecked(false);

    mapper->toFirst();
}

void MainWindow::selectVocabulary(int index)
{
    QString path("/home/taras/Projects/slovopedia.org.ua/%1/");
    path = path.arg(ui->comboBoxVocabulary->itemData(index, Qt::UserRole).toString());

    if (thread)
        thread->setPath(path, vocabularyId);
}

void MainWindow::updateStatusBar(QString str)
{
    ui->statusBar->showMessage(str);
}

void MainWindow::submitButton(void)
{
    bool ok = mapper->submit();

    if ( !ok )
        qDebug() << "submitButton: " << model->lastError().text();
}

void MainWindow::exportVocabulary(void)
{
    QSqlQuery query;
    QString fn = "/home/taras/Projects/vocabulary_%1.txt";
    int countRecord;

    fn = fn.arg(vocabularyId);

    QFile out(fn);

    QString q("SELECT _word, _question FROM slovopedia.works WHERE _vocabulary = %1;");
    q = q.arg(vocabularyId);

    query.exec(q);

    QSqlError le = query.lastError();
    if (le.type() == QSqlError::NoError)
    {
        if (query.isActive() && query.isSelect())
            countRecord = query.size();
        else
            countRecord = -1;

        if (countRecord > 0)
        {
            out.open(QIODevice::WriteOnly);

            int wordNo = query.record().indexOf("_word");
            int questionNo = query.record().indexOf("_question");

            while (query.next())
            {
                out.write("<word>");
                out.write(query.value(wordNo).toByteArray());
                out.write("</word>\n");

                out.write("<question>\n");
                out.write(query.value(questionNo).toByteArray());
                out.write("\n</question>\n\n");
            }

            out.close();
        }
    }
    else
        qDebug() << "exportVocabulary: " << le.text();
}
