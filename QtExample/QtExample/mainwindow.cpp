#include "mainwindow.h"
#include <QMessageBox>
//#include <QtXml/QtXml>

//#include<QtXml/QDomDocument>
#include <QFile>

MainWindow::MainWindow(std::unique_ptr<QWidget> parent) : QMainWindow(parent.get()),
                                                          ui(new Ui::MainWindowClass)
{
    ui->setupUi(this);

    //TO DO:make function
    ui->lineEditN->setValidator(new QRegExpValidator(QRegExp("0|(^[1-9][0-9]?$|^100$)"), this));
    ui->lineEditAddPoints->setValidator(new QRegExpValidator(QRegExp("^[0-9]+$"), this));
    ui->lineEditA->setValidator(new QRegExpValidator(QRegExp("^[0-9]+$"), this));
    ui->lineEditB->setValidator(new QRegExpValidator(QRegExp("^[0-9]+$"), this));
    ui->lineEditML->setValidator(new QRegExpValidator(QRegExp("^[0-9]+$"), this));
    ui->lineEditMU->setValidator(new QRegExpValidator(QRegExp("^[0-9]+$"), this));

    connect(ui->pushButtonStep1, &QPushButton::clicked, this, &MainWindow::SelectButtonStep1);
    connect(ui->pushButtonStep2, &QPushButton::released, this, &MainWindow::SelectButtonStep2);
    connect(ui->pushButtonAddPoints, &QPushButton::released, this, &MainWindow::AddPointsButton);
    connect(ui->pushButtonDefaultStep1, &QPushButton::released, this, &MainWindow::DefaultStep1);
    connect(ui->pushButtonDefaultStep2, &QPushButton::released, this, &MainWindow::DefaultStep2);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::ActionExit);
}

std::string MainWindow::replaceConstant(const std::string &input, const std::string &token, const std::string &token_value)
{
    std::string ecuation = input;
    std::size_t start = input.find(token);

    while (start != std::string::npos)
    {
        ecuation.replace(start, token.size(), token_value);

        start = ecuation.find(token);
    }

    return ecuation;
}

void MainWindow::replaceConstant(std::string &input, const std::unordered_map<std::string, std::string> &tokens)
{

    std::size_t start = std::string::npos;

    for (auto itr : tokens)
    {
        start = input.find(itr.first);
        while (start != std::string::npos)
        {
            input.replace(start, itr.first.size(), itr.second);

            start = input.find(itr.first);
        }
    }
}

double MainWindow::calculateExpression(CMathParser &mathParser, const std::string &line)
{
    double result;

    if (mathParser.Calculate(line.c_str(), &result) == CMathParser::ResultOk)
    {
        return result;
    }

    return NAN;
}

double MainWindow::generateKthTerm(CMathParser &mathParser, const QString &line, uint8_t k)
{
    std::string ecuation = line.toStdString();

    ecuation = replaceConstant(ecuation, "n", std::to_string(k));

    return calculateExpression(mathParser, ecuation);

    //replaceConstant(line, constants);
}

double MainWindow::generateFComp(CMathParser &mathParser, std::string &lineToEdit, double xComp_k, double xComp_k1, double yComp_k, double yComp_k1, double x, double y, uint8_t k)
{

    /*
    std::string ecuationComp(lineToEdit.toStdString());
    ecuationComp = replaceConstant(lineToEdit.toStdString(),"(XN)",std::move(std::to_string(comp_k)));
    ecuationComp = replaceConstant(ecuationComp,"(XN-1)",std::move(std::to_string(comp_k1)));
    ecuationComp = replaceConstant(ecuationComp,"(YN)",std::move(std::to_string(comp_k)));
    ecuationComp = replaceConstant(ecuationComp,"(YN-1)",std::move(std::to_string(comp_k1)));
    ecuationComp = replaceConstant(ecuationComp,"(M)",ui->lineEditMU->text().toStdString());
    ecuationComp = replaceConstant(ecuationComp,"m",ui->lineEditML->text().toStdString());
    ecuationComp = replaceConstant(ecuationComp,"a",ui->lineEditA->text().toStdString());
    ecuationComp = replaceConstant(ecuationComp,"b",ui->lineEditB->text().toStdString());
    ecuationComp = replaceConstant(ecuationComp,"(DN)",ui->lineEditDN->text().toStdString());
    ecuationComp = replaceConstant(ecuationComp,"X",std::move(std::to_string(x)));
    ecuationComp = replaceConstant(ecuationComp,"Y",std::move(std::to_string(y)));
    */

    std::string ecuationComp(lineToEdit);

    std::unordered_map<std::string, std::string> constants;
    constants.insert({"(XN)", std::to_string(xComp_k)});
    constants.insert({"(XN-1)", std::to_string(xComp_k1)});
    constants.insert({"(YN)", std::to_string(yComp_k)});
    constants.insert({"(YN-1)", std::to_string(yComp_k1)});
    constants.insert({"X", std::to_string(x)});
    constants.insert({"Y", std::to_string(y)});
    constants.insert({"n", std::to_string(k)});

    replaceConstant(ecuationComp, constants);

    return calculateExpression(mathParser, ecuationComp);
}

QCPGraphData MainWindow::generateFk(CMathParser &mathParser, uint8_t k, double x, double y, std::string &fX, std::string &fY)
{
    QCPGraphData resultPoint;

    resultPoint.key = generateFComp(mathParser, fX, xK[k], xK[k - 1], yK[k], yK[k - 1], x, y, k);

    resultPoint.value = generateFComp(mathParser, fY, xK[k], xK[k - 1], yK[k], yK[k - 1], x, y, k);

    return resultPoint;
}

QCPGraphData MainWindow::generate2DPoints(CMathParser &mathParser, std::string &fX, std::string &fY)
{

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> disDouble(0.0, 1.0);
    std::uniform_int_distribution<> disInt(1, 100);
    QCPGraphData result;
    uint8_t k = disInt(gen);

    double x0 = disDouble(gen);
    double y0 = disDouble(gen);

    return generateFk(mathParser, k, x0, y0, fX, fY);
    ;
}

void MainWindow::plotting(int numberFile)
{
    ui->potWidget->legend->clearItems();
    ui->potWidget->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // period as decimal separator and comma as thousand separator
    ui->potWidget->legend->setVisible(true);

    ui->potWidget->addGraph(ui->potWidget->yAxis2, ui->potWidget->xAxis2);
    // ui->potWidget->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 1));

    QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(9); // and make a bit smaller for legend
    ui->potWidget->legend->setFont(legendFont);
    ui->potWidget->legend->setBrush(QBrush(QColor(255, 255, 255, 1)));

    //ui->potWidget->addGraph(ui->potWidget->yAxis2, ui->potWidget->xAxis2);
    ui->potWidget->graph(0)->setPen(QColor(50, 50, 50, 255));
    // ui->potWidget->graph(0)->setLineStyle(QCPGraph::lsNone);

    // generate data, just playing with numbers, not much to learn here:
    QVector<std::pair<double, double>> points;

    double minX = DBL_MAX;
    double maxX = DBL_MIN;
    double minY = DBL_MAX;
    double maxY = DBL_MIN;

    auto readFile = [&](std::string fileName) {
        for (std::ifstream in(fileName); !in.eof();)
        {
            std::string line;
            std::getline(in, line);
            if (std::regex_match(line, std::regex(R"([+-]?(\d*.\d*) [+-]?(\d*.\d*))")))
            {
                std::stringstream ss(line);
                std::string item;
                std::getline(ss, item, ' ');
                
                double x = std::stod(item);

                if (x < minX)
                    minX = x;
                else if (x > maxX)
                    maxX = x;

                std::getline(ss, item, ' ');
                double y = std::stod(item);

                if (y < minY)
                    minY = y;
                else if (y > maxY)
                    maxY = y;
                
                points.push_back({y,x});
            }
        }
    };

    for(int index = 1; index<=numberFile; ++index)
        {
             readFile("..//.//QtExample\\Step1\\file"+std::to_string(index)+".txt");
        }

    auto comparator = [](const std::pair<double, double>& point1, const std::pair<double, double>& point2)
    {
        return point1.first < point2.first;
    };

    qSort(points.begin(), points.end(), comparator);

    QVector<double> xVector;
    QVector<double> yVector;

    for(auto& item : points)
    {
        xVector.push_back(std::move(item.first));
        yVector.push_back(std::move(item.second));
    }
    // pass data points to graphs:
    ui->potWidget->graph(0)->setData(xVector, yVector);
    // activate top and right axes, which are invisible by default:
    ui->potWidget->xAxis2->setVisible(true);
    ui->potWidget->yAxis2->setVisible(true);
    // set ranges appropriate to show data:
    ui->potWidget->xAxis->setRange(minX - PADDING, maxX + PADDING);
    ui->potWidget->xAxis2->setRange(minX - PADDING, maxX + PADDING);
    ui->potWidget->yAxis->setRange(minY - PADDING, maxY + PADDING);
    ui->potWidget->yAxis2->setRange(minY - PADDING, maxY + PADDING);
    // set labels:
    ui->potWidget->xAxis->setLabel("Bottom axis with outward ticks");
    ui->potWidget->yAxis->setLabel("Left axis label");
    ui->potWidget->xAxis2->setLabel("Top axis label");
    ui->potWidget->yAxis2->setLabel("Right axis label");

    ui->potWidget->replot();
}

void MainWindow::replaceFunction(std::string &line)
{
    std::unordered_map<std::string, std::string> constants;
    constants.insert({"(M)", ui->lineEditMU->text().toStdString()});
    constants.insert({"m", ui->lineEditML->text().toStdString()});
    constants.insert({"a", ui->lineEditA->text().toStdString()});
    constants.insert({"b", ui->lineEditB->text().toStdString()});
    constants.insert({"(DN)", ui->lineEditDN->text().toStdString()});

    replaceConstant(line, constants);
}

void MainWindow::generateKthTerms(CMathParser &parser)
{
    for (uint8_t index = 0; index <= 100; ++index)
    {
        xK.at(index) = generateKthTerm(parser, ui->lineEditXn->text(), index);
        yK.at(index) = generateKthTerm(parser, ui->lineEditYn->text(), index);
    }
}

void MainWindow::SelectButtonStep1()
{
    int numberPoint = 10000;
        int numberThread;
        if(std::thread::hardware_concurrency()>0) numberThread=std::thread::hardware_concurrency();
        else numberThread=4;
    CMathParser parser;

    //Generez toti xn,yn
    generateKthTerms(parser);

    std::string fX(ui->lineEditFx->text().toStdString()), fY(ui->lineEditFy->text().toStdString());

    replaceFunction(fX);
    replaceFunction(fY);

    auto generate = [&](int start, int stop, std::string fileName) {
        std::ofstream out(fileName);
        for (int i = start; i < stop; ++i) // data for graphs 2, 3 and 4
        {
            QCPGraphData temp = generate2DPoints(parser, fX, fY);
            out << temp.key << " " << temp.value << "\n";
        }
        out.close();
    };

    std::vector<std::thread> threads;

        auto spawnThreads=[&]()
        {
            for (int i = 1; i <= numberThread; i++) {
                threads.push_back( std::thread(generate, 0, numberPoint/numberThread, "..//.//QtExample\\Step1\\file"+std::to_string(i)+".txt"));
            }

            for (auto& th : threads) {
                th.join();
            }
        };

        spawnThreads();
        plotting(numberThread);

}

void MainWindow::SelectButtonStep2()
{
    CMathParser parser;

    generateKthTerms(parser);

    /*
    for(int index = 0; index < 10000; index++)
    {

    QCPGraphData temp = generate2DPoints(parser,fX,fY);
    s += QString::number(temp.key) + QString::number(temp.value);

    }
    ui->lineEditX0->setText(s);
    */
}

void MainWindow::AddPointsButton()
{
    int numberPoint = ui->lineEditAddPoints->text().toInt();
        int numberThread ;
        if(std::thread::hardware_concurrency()>0) numberThread=std::thread::hardware_concurrency();
        else numberThread=4;
    CMathParser parser;

    generateKthTerms(parser);

    std::string fX(ui->lineEditFx->text().toStdString()), fY(ui->lineEditFy->text().toStdString());

    replaceFunction(fX);
    replaceFunction(fY);

    auto generate = [&](int start, int stop, std::string fileName) {
        std::ofstream out(fileName, std::ios_base::app);
        for (int i = start; i < stop; ++i) // data for graphs 2, 3 and 4
        {
            QCPGraphData temp = generate2DPoints(parser, fX, fY);
            out << temp.key << " " << temp.value << "\n";
        }
        out.close();
    };
    std::vector<std::thread> threads;

        auto spawnThreads=[&]()
        {
            for (int i = 1; i <= numberThread; i++) {
                threads.push_back( std::thread(generate, 0, numberPoint/numberThread, "..//.//QtExample\\Step1\\file"+std::to_string(i)+".txt"));
            }

            for (auto& th : threads) {
                th.join();
            }
        };

        spawnThreads();
        plotting(numberThread);
}

void MainWindow::ActionExit()
{
    exit(1);
}

void MainWindow::DefaultStep1()
{
    /*
    QDomDocument doc("mydocument");
    QFile file("mydocument.xml");
    if (!file.open(QIODevice::ReadOnly))
        return;
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();
    */
}

void MainWindow::DefaultStep2()
{
}

MainWindow::~MainWindow()
{
    //EMPTY
}
