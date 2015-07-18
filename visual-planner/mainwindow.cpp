#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "kkplansql.h"

#include <QDebug>

#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QMouseEvent>
#include <QPoint>
#include <QMenu>
#include <QVBoxLayout>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    fieldPix = new QPixmap("image/double-sized-field.png");
    ui->field->setPixmap(*fieldPix);

    geometryIni();

    playOff = new playoff(this);
    playOff->setLabel(ui->field);
    playOff->setWidget(ui->POtabWidget);
    playOff->setLineEdits(ui->POTBPosX, ui->POTBPosY, ui->POTBPosAng, ui->POTBPosTol);
    playOff->setStatusBar(ui->statusBar);
    playOff->setAgentSizeCB(ui->comboBox_2);

    currentVPMode = kkPlayOn;

    this->setWindowTitle("Visual Planner");
    bRect[0].setRect(25, 25, 354, 605);
    bRect[1].setRect(379, 25, 455, 177);
    bRect[2].setRect(379, 202, 100, 249);
    bRect[3].setRect(479, 202, 255, 249);
    bRect[4].setRect(734, 202, 100, 249);
    bRect[5].setRect(379, 451, 455, 179);

    aRect[0].setRect(25, 25, 354, 605);
    aRect[1].setRect(379, 25, 100, 177);
    aRect[2].setRect(479, 25, 255, 177);
    aRect[3].setRect(734, 25, 100, 177);
    aRect[4].setRect(379, 202, 100, 249);
    aRect[5].setRect(479, 202, 255, 249);
    aRect[6].setRect(734, 202, 100, 249);
    aRect[7].setRect(379, 451, 100, 179);
    aRect[8].setRect(479, 451, 255, 179);
    aRect[9].setRect(734, 451, 100, 179);

    setQlabelInTabWidget(ui->tab_2, _1AgentLable, QRect(25,5,375,480),1);
    setQlabelInTabWidget(ui->tab_3, _2AgentLable, QRect(25,5,375,480),2);
    setQlabelInTabWidget(ui->tab_4, _3AgentLable, QRect(25,5,375,480),3);
    setQlabelInTabWidget(ui->tab_5, _4AgentLable, QRect(25,5,375,480),4);
    setQlabelInTabWidget(ui->tab_6, _5AgentLable, QRect(25,5,375,480),5);

    for(int i = 0; i < 5; i++)
        agentPlanCount[i] = 0;

    errorCheck = false;
    errorCounter = 0;

    addBtnMode = 0;

    lastPlan = -1;

    tabChanged = false;

    unsavedPlanFlag = false;

    currentPlan = new kkAgentPlanClass();

    myPlanSql =  new kkPlanSQL("temp.db");

    createActions();
    POCreateActions();

    bAreaSelected = false;
    bTempArea = bSelectedArea = -1;

    on_ballPossessionCombo_currentIndexChanged(ui->ballPossessionCombo->currentIndex());
    on_endPolicyValTextBox_editingFinished();
    on_endPolicyValCombo_currentIndexChanged(ui->endPolicyValCombo->currentIndex());
    on_endPolicyCombo_currentIndexChanged(ui->endPolicyCombo->currentIndex());

    for(int i = 0; i < 5; i++)
        aTempArea[i][0] = aTempArea[i][1] = aSelectedArea[i][0] = aSelectedArea[i][1] = -1;
    aAreaSelected = false;
    currentAgent = 0;
    currentPriority = 0;
    aOrB = 0;

    rightClickEmpty = false;

    mode = BALL;
    lastMode = mode;
    //showBallArea(bSelectedArea);
    on_tabWidget_currentChanged(ui->tabWidget->currentIndex());
    currentTab = ui->tabWidget->currentIndex();

    changeBallScroll();

    currentPlan->clear(1);
    insertPlanToStruct(unsavedPlan);

    timer = new QTimer();
    timer->start(100);
    timerCounter = 0;
    blinkCurrentAgent = true;
    connect(timer, SIGNAL(timeout()), this, SLOT(timerSlot()));

    POCurrentPlan = 0;

    ui->newBtn->setVisible(false);
    POSpinInit = true;

}

PEndPolicy MainWindow::getPolicyByIndex(int index)
{
    switch(index)
    {
        default:
        case 1:
            return Cycle;
        break;
        case 2:
            return ExactAgent;
        break;
        case 3:
            return AllAgents;
        break;
        case 4:
            return ExactDisturb;
        break;
    }
}

int MainWindow::getIntByEnum(PEndPolicy tEnum)
{
    switch (tEnum)
    {
        case Cycle:
            return 1;
        case ExactAgent:
            return 2;
        case AllAgents:
            return 3;
        case ExactDisturb:
            return 4;
    }
}

PSkills MainWindow::getPSkillByIndex(int index)
{
    switch (index)
    {
        case 0:
            return None;
        case 1:
            return MoveOffensive;
        case 2:
            return MoveDefensive;
        case 3:
            return PassOffensive;
        case 4:
            return PassDefensive;
        case 5:
            return KickOffensive;
        case 6:
            return KickDefensive;
        case 7:
            return ChipOffensive;
        case 8:
            return ChipDefensive;
        case 9:
            return MarkOffensive;
        case 10:
            return MarkDefensive;
        case 11:
            return OneTouch;
        case 12:
            return CatchBall;
        case 13:
            return ReceivePass;
        case 14:
            return Shot;
        case 15:
            return ChipToGoal;
    }
}

int MainWindow::getIntByEnum(PSkills tEnum)
{
    switch (tEnum)
    {
        case None:
            return 0;
        case MoveOffensive:
            return 1;
        case MoveDefensive:
            return 2;
        case PassOffensive:
            return 3;
        case PassDefensive:
            return 4;
        case KickOffensive:
            return 5;
        case KickDefensive:
            return 6;
        case ChipOffensive:
            return 7;
        case ChipDefensive:
            return 8;
        case MarkOffensive:
            return 9;
        case MarkDefensive:
            return 10;
        case OneTouch:
            return 11;
        case CatchBall:
            return 12;
        case ReceivePass:
            return 13;
        case Shot:
            return 14;
        case ChipToGoal:
            return 15;
    }
}

int MainWindow::getIntByEnum(PlannerMode tEnum)
{
      switch (tEnum)
      {
          case BALL:
              return 0;
          case _1AGENT:
              return 1;
          case _2AGENT:
              return 2;
          case _3AGENT:
              return 3;
          case _4AGENT:
              return 4;
          case _5AGENT:
              return 5;
      }
}

//This timer is for animating selected agent in PlayOn mode
//Selected agent suppose to blink
void MainWindow::timerSlot()
{
    if(currentVPMode == kkPlayOn)
    {
        timerCounter++;
        if(timerCounter > 10) timerCounter = 0;
        if(aAreaSelected && !blinkCurrentAgent)
        {
            blinkCurrentAgent = true;
            showAgentArea(aSelectedArea[currentAgent][aOrB]);
        }
        if(mode != BALL && !aAreaSelected)
        {
            if(timerCounter == 6)
            {
                blinkCurrentAgent = false;
                showAgentArea(aSelectedArea[currentAgent][aOrB]);
            }
            if(timerCounter == 0)
            {
                blinkCurrentAgent = true;
                showAgentArea(aSelectedArea[currentAgent][aOrB]);
            }
        }
        if(errorCheck)
        {
            errorCounter++;
            if(errorCounter > 30)
            {
                errorCounter = 0;
                errorCheck = false;
                ui->statusBar->setStyleSheet("");
            }
        }
    }

}

//Setting skills button for PalyOn
//For making it more stylish, it has been implimented using QLable
void MainWindow::setQLabel(QWidget *widget, QLabel **label, QRect rect, int index)
{
    //skill lables
    label[index*3] = new QLabel(widget);
    label[index*3]->setGeometry(QRect(rect.left(), rect.top(), rect.width(), (rect.height()*2)/3 ));
    label[index*3]->setText(QString::number(index%4 + 1)+": NA");
    label[index*3]->setStyleSheet("QLabel { background-color : #0866af; color : white; font-weight: bold;} QLabel:HOVER { background-color : #2f78b3; }");
    label[index*3]->setAlignment(Qt::AlignCenter|Qt::AlignHCenter);
    label[index*3]->setCursor(Qt::PointingHandCursor);

    //A point lables
    label[index*3 + 1] = new QLabel(widget);
    label[index*3 + 1]->setGeometry(QRect(rect.left(), rect.top() + (rect.height()*2)/3 ,rect.width()/2, rect.height()/3 ));
    label[index*3 + 1]->setText("A:NA");
    label[index*3 + 1]->setStyleSheet("QLabel { background-color : #ea8c00; color : white; font-weight: bold;} QLabel:HOVER { background-color : #dfa752; }");
    label[index*3 + 1]->setAlignment(Qt::AlignCenter|Qt::AlignHCenter);
    label[index*3 + 1]->setCursor(Qt::PointingHandCursor);

    //B point lables
    label[index*3 + 2] = new QLabel(widget);
    label[index*3 + 2]->setGeometry(QRect(rect.left() + rect.width()/2, rect.top() + (rect.height()*2)/3, rect.width()/2, rect.height()/3 ));
    label[index*3 + 2]->setText("B:NA");
    label[index*3 + 2]->setStyleSheet("QLabel { background-color : #0fad00; color : white; font-weight: bold;} QLabel:HOVER { background-color : #39ad2e; }");
    label[index*3 + 2]->setAlignment(Qt::AlignCenter|Qt::AlignHCenter);
    label[index*3 + 2]->setCursor(Qt::PointingHandCursor);
}

//this function set geometry of lables
void MainWindow::setAgentLabel(QWidget *widget, QRect rect, int size)
{
    int paddingH = 3;
    int from, to;
    switch(size)
    {
        default:
        case 1:
          from = 0;
          to = 1;
        break;
        case 2:
          from = 1;
          to = 3;
        break;
        case 3:
          from = 3;
          to = 6;
        break;
        case 4:
          from = 6;
          to = 10;
        break;
        case 5:
          from = 10;
          to = 15;
        break;
    }

    for(int i = from; i < to; i++)
    {
        agentLabel[i] = new QLabel(widget);
        agentLabel[i]->setGeometry(QRect(rect.x(), rect.y() + (rect.height()+paddingH) * (i-from), rect.width(), rect.height()-1));
        agentLabel[i]->setText("A\n#\n"+QString::number((i - from) + 1));
        agentLabel[i]->setStyleSheet("QLabel { background-color : #f30c03; color : white; font-weight: bold;}");
        agentLabel[i]->setAlignment(Qt::AlignCenter|Qt::AlignHCenter);
    }
}

//Sets lables in PlayOn TabWidget
void MainWindow::setQlabelInTabWidget(QWidget *widget, QLabel **label, QRect rect, int size)
{
    int fWidth = rect.width(), fHeight = rect.height(), fX = rect.left(), fY = rect.top();
    int blockHeight = 80, paddingW = 3, paddingH = 3;
    int blockWidth = fWidth / 4 - paddingW;
    QRect tempRect;
    setAgentLabel(widget,QRect(rect.x()-20, rect.y(), 20, blockHeight),size);
    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            tempRect.setRect(fX + j * (blockWidth + paddingW), fY + i * (blockHeight + paddingH), blockWidth, blockHeight);
            setQLabel(widget, label, tempRect, i * 4 + j);
        }
    }
}

//initializing widgets geometries
//this allows user to have cross platform availability with same style
//basicalluy ignores the OS margin, paddings and etc.
void MainWindow::geometryIni()
{
    int width = ui->centralWidget->width(), height = ui->centralWidget->height()-30;
    QRect fieldRect(5, 5, 859, 655), tabWidgetRect(874, 5, 401, 655);
    ui->field->setGeometry(fieldRect);
    ui->tabWidget->setGeometry(tabWidgetRect);

    ui->POtabWidget->setGeometry(tabWidgetRect);
    ui->POtabWidget->setVisible(false);

    //////////
    QRect ballPosLabelRect(10, 10, 120 , 30);
    QRect ballPosComboRect(135, 10, 150, 30);
    ui->ballPosLabel->setGeometry(ballPosLabelRect);
    ui->ballPosCombo->setGeometry(ballPosComboRect);
    QRect ballPossessionLabelRect(10, 45, 120, 30);
    QRect ballPossessionComboRect(135, 45, 150, 30);
    ui->ballPossessionLabel->setGeometry(ballPossessionLabelRect);
    ui->ballPossessionCombo->setGeometry(ballPossessionComboRect);
    //////////
    QRect groupBoxRect(10, 85, 390, 95);
    QRect policyModeLabelRect(10, 20, 100, 30);
    QRect policyModeComboRect(125, 20, 150, 30);
    QRect policyValLabelRect(10, 55, 100, 30);
    QRect policyValComboRect(125, 55, 150, 30);
    QRect policyValTextBoxRect(125, 55, 150, 30);
    ui->groupBox->setGeometry(groupBoxRect);
    ui->endPolicyLabel->setGeometry(policyModeLabelRect);
    ui->endPolicyCombo->setGeometry(policyModeComboRect);
    ui->endPolicyValLabel->setGeometry(policyValLabelRect);
    ui->endPolicyValCombo->setGeometry(policyValComboRect);
    ui->endPolicyValTextBox->setGeometry(policyValTextBoxRect);
    ui->endPolicyValCombo->setVisible(false);
    ////////////
    QRect planSizeRect(10, 175, 150, 30);
    QRect agentSizeRect(10, 205, 150, 30);
    QRect commentTextBoxRect(10, 250, 377, 360);
    ui->planSizeLabel->setGeometry(planSizeRect);
    ui->agentSizeLabel->setGeometry(agentSizeRect);
    ui->commentTextBox->setGeometry(commentTextBoxRect);
    //////////
    QRect browseBtnRect(5, 665, 100, 25);
    QRect spinRect(110, 665, 70, 25);
    QRect addBtnRect(185, 665, 100, 25);
    QRect resetBtnRect(290, 665, 100, 25);
    QRect saveBtnRect(395, 665, 100, 25);
    QRect removeBtnRect(500, 665, 100, 25);
    QRect newBtnRect(605, 665, 100, 25);
    ui->browseBtn->setGeometry(browseBtnRect);
    ui->spinBox->setGeometry(spinRect);
    ui->addBtn->setGeometry(addBtnRect);
    ui->resetBtn->setGeometry(resetBtnRect);
    ui->saveBtn->setGeometry(saveBtnRect);
    ui->removeBtn->setGeometry(removeBtnRect);
    ui->newBtn->setGeometry(newBtnRect);
    ////////
    QRect ballLabelRect(874, 662, 400, 33);
    ui->ballLabel->setGeometry(ballLabelRect);
    ui->ballLabel->setStyleSheet("QLabel {background-color: orange; color: #fff; font-weight: bold;}");
    ui->ballLabel->setAlignment(Qt::AlignCenter|Qt::AlignHCenter);
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    geometryIni();
}

//Sets ball position using QCombobox
void MainWindow::changeBallScroll()
{
    ui->ballPosCombo->setCurrentIndex((bSelectedArea != -1)? bSelectedArea-1:6);
}

//Get selected area in PlayOn Mode
//in ball mode it varies from 1 to 6
//in agent mode it varies from 1 to 10
int MainWindow::getArea(QRect *rect, int size, QPoint mPos)
{
    if(addBtnMode != 2)
    {
        ui->addBtn->setText("Apply");
        addBtnMode = 0;
    }
    for(int i = 0; i < size; i++)
    {
        if(rect[i].contains(mPos))
            return i + 1;
    }
    return -1;
}

//since this appliation uses Qlable instead of buttons-
//it uses mouse event to find out which QLable is selected
kkLabelMode MainWindow::getPressedLabel(QLabel **label, int size)
{
    kkLabelMode temp;
    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            for(int k = 0; k < 3; k++)
            {
                if(label[k + j * 3 + i * 12 ]->underMouse())
                {
                    temp.agent = i;
                    temp.priority = j;
                    temp.part = k;
                    return temp;
                }
            }
        }
    }
    temp.agent = -1;
    temp.priority = -1;
    temp.part = -1;
    return temp;
}

//Up
kkLabelMode MainWindow::getPressedLabel()
{
    switch(mode)
    {
        case _1AGENT:
            return getPressedLabel(_1AgentLable, 1);
        break;
        case _2AGENT:
            return getPressedLabel(_2AgentLable, 2);
        break;
        case _3AGENT:
            return getPressedLabel(_3AgentLable, 3);
        break;
        case _4AGENT:
            return getPressedLabel(_4AgentLable, 4);
        break;
        case _5AGENT:
            return getPressedLabel(_5AgentLable, 5);
        break;
    }
}

//Set A and B points text
void MainWindow::setLabelABText(QLabel **label, int size)
{
    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            for(int k = 1; k < 3; k++)
            {
                if(k == 1)
                    label[k + j * 3 + i * 12 ]->setText("A:"+((aSelectedArea[i][0] != -1)? QString::number(aSelectedArea[i][0]):"NA"));
                else
                    label[k + j * 3 + i * 12 ]->setText("B:"+((aSelectedArea[i][1] != -1)? QString::number(aSelectedArea[i][1]):"NA"));
            }
        }
    }
}

//up
void MainWindow::setLabelABText()
{
    switch(mode)
    {
        case _1AGENT:
            setLabelABText(_1AgentLable, 1);
        break;
        case _2AGENT:
            setLabelABText(_2AgentLable, 2);
        break;
        case _3AGENT:
            setLabelABText(_3AgentLable, 3);
        break;
        case _4AGENT:
            setLabelABText(_4AgentLable, 4);
        break;
        case _5AGENT:
            setLabelABText(_5AgentLable, 5);
        break;
    }
}

//mouse press event
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QPoint tempPos;
    tempPos = ui->centralWidget->mapFromParent(event->pos());
    tempPos = ui->field->mapFromParent(tempPos);
    //PlayOn Mode
    if(currentVPMode == kkPlayOn)
    {
        if(event->buttons() == Qt::LeftButton)
        {
            //checks if mouse is in field QLable
            if(ui->field->underMouse())
            {
                //gets ball position
                if(mode == BALL)
                {
                    bAreaSelected = true;
                    bTempArea = getArea(bRect, 6, tempPos);
                    if(bSelectedArea != bTempArea)
                        showBallArea(bTempArea);
                }
                else//gets agent positions
                {
                    aAreaSelected = true;
                    aTempArea[currentAgent][aOrB] = getArea(aRect, 10, tempPos);
                    if(aSelectedArea[currentAgent][aOrB] != aTempArea[currentAgent][aOrB])
                        showAgentArea(aTempArea[currentAgent][aOrB]);
                }
            }
            //cheks if mouse is PlayOn tabWidget
            else if(ui->tabWidget->underMouse())
            {
                kkLabelMode tempLabel = getPressedLabel();
                if(tempLabel.part != -1 && tempLabel.priority != -1 && tempLabel.agent != -1 && mode != BALL)
                {
                    if(tempLabel.priority != 0)
                        if(currentPlan->getAgentSkill(currentAgent + 1, tempLabel.priority) == None &&
                           currentPriority < tempLabel.priority)
                            return;
                    if(tempLabel.part == 1 || (tempLabel.part == 0 && currentAgent != tempLabel.agent))
                        aOrB = 0;
                    else if(tempLabel.part == 2)
                        aOrB = 1;
                    currentAgent = tempLabel.agent;
                    showAgentArea(aSelectedArea[currentAgent][aOrB]);
                    setLabelABText();
                    currentAgent = tempLabel.agent;
                    currentPriority = tempLabel.priority;
                    setSkillColors(currentAgent, currentPriority, aOrB);
                }
            }
        }
        //right click removes skills
        else if(event->buttons() == Qt::RightButton)
        {
            if(ui->tabWidget->underMouse())
            {
                rightClickEmpty = true;
            }
        }
    }
    //PlayOff mode
    //For making it easy PlayOff functions is implemented in another class
    else if(currentVPMode == kkPlayOff)
    {
        playOff->mousePressed(event, tempPos);
    }
}

//mouse release event
//this is mainly for deactiving acrtived event in Mouse Press Event
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint tempPos;
    tempPos = ui->centralWidget->mapFromParent(event->pos());
    tempPos = ui->field->mapFromParent(tempPos);
    if(currentVPMode == kkPlayOn)
    {
        if(ui->field->underMouse())
        {
            if(bAreaSelected)
            {
                bSelectedArea = getArea(bRect, 6, tempPos);
                if(bSelectedArea != bTempArea)
                    showBallArea(bSelectedArea);
                changeBallScroll();
                currentPlan->setBallPos(bSelectedArea);
            }
            else if(aAreaSelected)
            {
                aSelectedArea[currentAgent][aOrB] = getArea(aRect, 10, tempPos);
                //if(aSelectedArea[currentAgent][aOrB] != aTempArea[currentAgent][aOrB])
                showAgentArea(aSelectedArea[currentAgent][aOrB]);
                setLabelABText();
                setPlanPoints();
                if(aOrB == 0)
                    activeB();
            }
            bAreaSelected = false;
            aAreaSelected = false;
        }
        else if(ui->tabWidget->underMouse())
        {
            if(rightClickEmpty)
            {
                kkLabelMode tempLabel = getPressedLabel();
                if(tempLabel.part != -1 && tempLabel.priority != -1 && tempLabel.agent != -1)
                {
                    emptySkill(tempLabel.agent, tempLabel.priority);
                }
                rightClickEmpty = false;
            }
        }
        else
        {
            if(bAreaSelected)
            {
                bTempArea = -1;
                showBallArea();
            }
            else if(aAreaSelected)
            {
                aTempArea[currentAgent][aOrB] = -1;
                showAgentArea();
                setLabelABText();
            }
            bAreaSelected = false;
            aAreaSelected = false;
        }
    }
    else if(currentVPMode == kkPlayOff)
    {
        playOff->mouseReleased(event, tempPos);
    }
}

//Mouse Move event
//this is added to have better animation
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint tempPos;
    tempPos = ui->centralWidget->mapFromParent(event->pos());
    tempPos = ui->field->mapFromParent(tempPos);
    if(currentVPMode == kkPlayOn)
    {
        int tempArea;
        if(ui->field->underMouse())
        {
            if(bAreaSelected)
            {
                tempArea = getArea(bRect, 6, tempPos);
                if(tempArea != bTempArea)
                    showBallArea(tempArea);
                bTempArea = tempArea;
            }
            else if(aAreaSelected)
            {
                tempArea = getArea(aRect, 10, tempPos);
                if(tempArea != aTempArea[currentAgent][aOrB])
                    showAgentArea(tempArea);
                aTempArea[currentAgent][aOrB] = tempArea;
            }
        }
    }
    else if(currentVPMode == kkPlayOff)
    {
        playOff->mouseMoved(event, tempPos);
    }
}

//Key Press Event
//Mainly for using shortcuts
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(currentVPMode == kkPlayOn)
    {
        if(mode != BALL)
            switch(event->key())
            {
                case Qt::Key_W:
                    activePrevAgent();
                break;
                case Qt::Key_S:
                    activeNextAgent();
                break;
                case Qt::Key_A:
                    activePrevPriority();
                break;
                case Qt::Key_D:
                    activeNextPriority();
                break;
                case Qt::Key_Q:
                    activeA();
                break;
                case Qt::Key_E:
                    activeB();
                break;
            }
        switch(event->key())
        {
            case Qt::Key_O:
            case Qt::Key_Up:
                ui->spinBox->setValue(ui->spinBox->value() + 1);
            break;
            case Qt::Key_L:
            case Qt::Key_Down:
                ui->spinBox->setValue(ui->spinBox->value() - 1);
            break;
            case Qt::Key_K:
            case Qt::Key_Left:
                currentTab--;
                if(currentTab < 0)
                    currentTab = 0;
                ui->tabWidget->setCurrentIndex(currentTab);
            break;
            case Qt::Key_Semicolon:
            case Qt::Key_Right:
                currentTab++;
                if(currentTab > 5)
                    currentTab = 5;
                ui->tabWidget->setCurrentIndex(currentTab);
            break;
            case Qt::Key_B:
                ui->tabWidget->setCurrentIndex(0);
            break;
        }
    }
    else if(currentVPMode == kkPlayOff)
    {
        if(event->modifiers() != Qt::ControlModifier)
        {
            switch(event->key())
            {
                case Qt::Key_W:
                case Qt::Key_Up:
                    if(ui->spinBox->value() + 1 <= ui->spinBox->maximum() )
                        ui->spinBox->setValue(ui->spinBox->value() + 1);
                break;
                case Qt::Key_S:
                case Qt::Key_Down:
                    if(ui->spinBox->value() - 1 >= 0 )
                        ui->spinBox->setValue(ui->spinBox->value() - 1);
                break;
            }
        }
        else if(event->modifiers() == Qt::ControlModifier)
        {
            switch(event->key())
            {
                case Qt::Key_O:
                    on_browseBtn_clicked();
                break;
                case Qt::Key_S:
                    on_saveBtn_clicked();
                break;
            }
        }
    }
}

//This fucntion creates actions for contex menu (aka right click menu)
void MainWindow::createActions()
{
    passSkill[0] = new QAction(tr("&Pass"), this);
    passSkill[0]->setStatusTip(tr("Set point A then B to send pass. Offensive"));
    connect(passSkill[0], SIGNAL(triggered()), this, SLOT(activePassO()));

    passSkill[1] = new QAction(tr("&Pass"), this);
    passSkill[1]->setStatusTip(tr("Set point A then B to send pass. Defensive"));
    connect(passSkill[1], SIGNAL(triggered()), this, SLOT(activePassD()));

    moveSkill[0] = new QAction(tr("&Move"), this);
    moveSkill[0]->setStatusTip(tr("Set point A then B to move. Offensive"));
    connect(moveSkill[0], SIGNAL(triggered()), this, SLOT(activeMoveO()));

    moveSkill[1] = new QAction(tr("&Move"), this);
    moveSkill[1]->setStatusTip(tr("Set point A then B to move. Defensive"));
    connect(moveSkill[1], SIGNAL(triggered()), this, SLOT(activeMoveD()));

    kickSkill[0] = new QAction(tr("&Kick"), this);
    kickSkill[0]->setStatusTip(tr("Set point A then B to kick. Defensive"));
    connect(kickSkill[0], SIGNAL(triggered()), this, SLOT(activeKickO()));

    kickSkill[1] = new QAction(tr("&Kick"), this);
    kickSkill[1]->setStatusTip(tr("Set point A then B to kick. Defensive"));
    connect(kickSkill[1], SIGNAL(triggered()), this, SLOT(activeKickD()));

    chipSkill[0] = new QAction(tr("&Chip"), this);
    chipSkill[0]->setStatusTip(tr("Set point A then B to chip. Offensive"));
    connect(chipSkill[0], SIGNAL(triggered()), this, SLOT(activeChipO()));

    chipSkill[1] = new QAction(tr("&Chip"), this);
    chipSkill[1]->setStatusTip(tr("Set point A then B to chip. Defensive"));
    connect(chipSkill[1], SIGNAL(triggered()), this, SLOT(activeChipD()));

    markSkill[0] = new QAction(tr("&Mark"), this);
    markSkill[0]->setStatusTip(tr("Set a region to Mark. Offensive"));
    connect(markSkill[0], SIGNAL(triggered()), this, SLOT(activeMarkO()));

    markSkill[1] = new QAction(tr("&Mark"), this);
    markSkill[1]->setStatusTip(tr("Set a region to Mark. Defensive"));
    connect(markSkill[1], SIGNAL(triggered()), this, SLOT(activeMarkD()));

    oneTouchSkill = new QAction(tr("&On Touch"), this);
    oneTouchSkill->setStatusTip(tr("Set point A then B to One Touch. Offensive"));
    connect(oneTouchSkill, SIGNAL(triggered()), this, SLOT(activeOneTouch()));

    catchBallSkill = new QAction(tr("&Catch Ball"), this);
    catchBallSkill->setStatusTip(tr("No need to select a region"));
    connect(catchBallSkill, SIGNAL(triggered()), this, SLOT(activeCatchBall()));

    receivePass = new QAction(tr("&Receive Pass"), this);
    receivePass->setStatusTip(tr("Set point A to receive pass (B point in pass skill)"));
    connect(receivePass, SIGNAL(triggered()), this, SLOT(activeReceivePass()));

    shotSkill = new QAction(tr("&Shot"), this);
    shotSkill->setStatusTip(tr("Set point to shot, B point is Goal"));
    connect(shotSkill, SIGNAL(triggered()), this, SLOT(activeShot()));

    chipToGoalSkill = new QAction(tr("&Chip to Goal"), this);
    chipToGoalSkill->setStatusTip(tr("Set point to chip, B point is behind opponent Defenders"));
    connect(chipToGoalSkill, SIGNAL(triggered()), this, SLOT(activeChipToGoal()));

    prevAgent = new QAction(tr("&Previous Agent"), this);
    prevAgent->setStatusTip(tr("Previous Agent"));
    connect(prevAgent, SIGNAL(triggered()), this, SLOT(activePrevAgent()));

    nextAgent = new QAction(tr("&Next Agent"), this);
    nextAgent->setStatusTip(tr("Next Agent"));
    connect(nextAgent, SIGNAL(triggered()), this, SLOT(activeNextAgent()));

    prevPriority = new QAction(tr("&Previous Priority"), this);
    prevPriority->setStatusTip(tr("Previous Priority"));
    connect(prevPriority, SIGNAL(triggered()), this, SLOT(activePrevPriority()));

    nextPriority = new QAction(tr("&Next Priority"), this);
    nextPriority->setStatusTip(tr("Next Priority"));
    connect(nextPriority, SIGNAL(triggered()), this, SLOT(activeNextPriority()));

    prevPriority->setEnabled(false);
    enableContexAgent();

}

//animates Qlables to make it more responsive
void MainWindow::setSkillColor(QLabel **label, int tI, int agent, int priority, int tAOrB)
{
    for(int i = 0; i < tI; i++)
    {
        for(int j = 0; j < 4; j++)
        {
              if(agent == i && priority == j)
              {
                    label[i*12 + j*3]->setStyleSheet("QLabel { background-color : #0866af; color : white; font-weight: bold;} QLabel:HOVER { background-color : #2f78b3; }");
                    if(tAOrB)
                    {
                        label[i*12 + j*3 + 1]->setStyleSheet("QLabel { background-color : #ea8c00; color : white; font-weight: bold;} QLabel:HOVER { background-color : #dfa752; }");
                        label[i*12 + j*3 + 2]->setStyleSheet("QLabel { background-color : #0fad00; color : #ffff00; font-weight: bold;} QLabel:HOVER { background-color : #39ad2e; }");
                    }
                    else
                    {
                        label[i*12 + j*3 + 1]->setStyleSheet("QLabel { background-color : #ea8c00; color : #ffff00; font-weight: bold;} QLabel:HOVER { background-color : #dfa752; }");
                        label[i*12 + j*3 + 2]->setStyleSheet("QLabel { background-color : #0fad00; color : white; font-weight: bold;} QLabel:HOVER { background-color : #39ad2e; }");
                    }
              }
              else
              {
                    label[i*12 + j*3]->setStyleSheet("QLabel { background-color : #89a1b5; color : white; font-weight: bold;} QLabel:HOVER { background-color : #2f78b3; }");
                    label[i*12 + j*3 + 1]->setStyleSheet("QLabel { background-color : #eec482; color : white; font-weight: bold;} QLabel:HOVER { background-color : #dfa752; }");
                    label[i*12 + j*3 + 2]->setStyleSheet("QLabel { background-color : #6dad67; color : white; font-weight: bold;} QLabel:HOVER { background-color : #39ad2e; }");
              }
        }
    }
}

//animates Qlables to make it more responsive
void MainWindow::setSkillColors(int agent, int priority, int tAOrB)
{
    switch(mode)
    {
          default:
          case _1AGENT:
                setSkillColor(_1AgentLable, 1, agent, priority, tAOrB);
          break;
          case _2AGENT:
                setSkillColor(_2AgentLable, 2, agent, priority, tAOrB);
          break;
          case _3AGENT:
                setSkillColor(_3AgentLable, 3, agent, priority, tAOrB);
          break;
          case _4AGENT:
                setSkillColor(_4AgentLable, 4, agent, priority, tAOrB);
          break;
          case _5AGENT:
                setSkillColor(_5AgentLable, 5, agent, priority, tAOrB);
          break;
    }
}

//converts skills enum to string
QString MainWindow::getSkillTextByEnum(PSkills skill)
{
    switch(skill)
    {
          default:
          case None:
              return "NA";
          break;
          case MoveOffensive:
              return "Move\nOffensive";
          break;
          case MoveDefensive:
              return "Move\nDefensive";
          break;
          case PassOffensive:
              return "Pass\nOffensive";
          break;
          case PassDefensive:
              return "Pass\nDefensive";
          break;
          case KickOffensive:
              return "Kick\nOffensive";
          break;
          case KickDefensive:
              return "Kick\nDefensive";
          break;
          case ChipOffensive:
              return "Chip\nOffensive";
          break;
          case ChipDefensive:
              return "Chip\nDefensive";
          break;
          case MarkOffensive:
              return "Mark\nOffensive";
          break;
          case MarkDefensive:
              return "Mark\nDefensive";
          break;
          case OneTouch:
              return "One\nTouch";
          break;
          case CatchBall:
              return "Catch\nBall";
          break;
          case ReceivePass:
              return "Receive\nPass";
          break;
          case Shot:
              return "Shot";
          break;
          case ChipToGoal:
              return "Chip to\nGoal";
          break;
    }
}

void MainWindow::setSkillText(QLabel **label, int tI)
{
    for(int i = 0; i < tI; i++)
    {
        for(int j = 0; j < 4; j++)
        {
              label[i*12 + j*3]->setText(QString::number(j + 1)+": "+getSkillTextByEnum(currentPlan->getAgentSkill(i + 1, j + 1)) );
        }
    }
}

void MainWindow::setSkillTexts()
{
    switch(mode)
    {
          default:
          case _1AGENT:
                setSkillText(_1AgentLable, 1);
          break;
          case _2AGENT:
                setSkillText(_2AgentLable, 2);
          break;
          case _3AGENT:
                setSkillText(_3AgentLable, 3);
          break;
          case _4AGENT:
                setSkillText(_4AgentLable, 4);
          break;
          case _5AGENT:
                setSkillText(_5AgentLable, 5);
          break;
    }
}

//checks if contex menu can have next and previous agent actions and ...
void MainWindow::enableContexAgent()
{
    switch(mode)
    {
         default:
         case _1AGENT:
             currentAgent = 0;
             prevAgent->setEnabled(false);
             nextAgent->setEnabled(false);;
         break;
         case _2AGENT:
             if(currentAgent>1) currentAgent = 1;
             if(currentAgent == 1)
                    nextAgent->setEnabled(false);
             else
                    nextAgent->setEnabled(true);
         break;
         case _3AGENT:
             if(currentAgent>2) currentAgent = 2;
             if(currentAgent == 2)
                    nextAgent->setEnabled(false);
             else
                    nextAgent->setEnabled(true);
         break;
         case _4AGENT:
             if(currentAgent>3) currentAgent = 3;
             if(currentAgent == 3)
                    nextAgent->setEnabled(false);
             else
                    nextAgent->setEnabled(true);
         break;
         case _5AGENT:
             if(currentAgent>4) currentAgent = 4;
             if(currentAgent == 4)
                    nextAgent->setEnabled(false);
             else
                    nextAgent->setEnabled(true);
         break;
    }

    if(mode == _1AGENT)
         prevAgent->setEnabled(false);
    else
    {
         if(currentAgent == 0)
              prevAgent->setEnabled(false);
         else
              prevAgent->setEnabled(true);
    }
}

//contex menu event/ it will be called if right click pressed
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    if(currentVPMode == kkPlayOn)
    {
        if(ui->field->underMouse() && mode != BALL)
        {
            QMenu *offMenu, *defMenu;
            QMenu menu(this);
            offMenu = menu.addMenu(tr("&Offensive"));
                    offMenu->addAction(moveSkill[0]);
                    offMenu->addAction(passSkill[0]);
                    offMenu->addAction(kickSkill[0]);
                    offMenu->addAction(chipSkill[0]);
                    offMenu->addAction(markSkill[0]);

            defMenu = menu.addMenu(tr("&Defensive"));
                    defMenu->addAction(moveSkill[1]);
                    defMenu->addAction(passSkill[1]);
                    defMenu->addAction(kickSkill[1]);
                    defMenu->addAction(chipSkill[1]);
                    defMenu->addAction(markSkill[1]);

            menu.addAction(receivePass);
            menu.addAction(oneTouchSkill);
            menu.addAction(shotSkill);
            menu.addAction(chipToGoalSkill);
            menu.addAction(catchBallSkill);
            menu.addSeparator();
            menu.addAction(prevAgent);
            menu.addAction(nextAgent);
            menu.addSeparator();
            menu.addAction(prevPriority);
            menu.addAction(nextPriority);

            menu.exec(event->globalPos());
        }
    }
    else
    {
        if(ui->field->underMouse())
        {
            qDebug()<<"check";
            QMenu menu(this);
            menu.addAction(POMove);
            menu.addSeparator();
            menu.addAction(POPass);
            menu.addAction(POReceivePass);
            menu.addAction(POShotToGoal);
            menu.addAction(POChipToGoal);
            menu.addAction(POOneTouch);

            menu.exec(event->globalPos());
        }
    }
}

//Set Skills for exact PlayOn plan
bool MainWindow::setPlan(PSkills skill)
{
    bool temp;
    temp = currentPlan->setAgentSkill(currentAgent + 1,
                                      currentPriority + 1,
                                      aSelectedArea[currentAgent][0],
                                      aSelectedArea[currentAgent][1],
                                      skill);
    setSkillTexts();
    if(addBtnMode != 2)
    {
        addBtnMode = 0;
        ui->addBtn->setText("Apply");
    }
    return temp;
}

//Set desired points for PlayOn skill
bool MainWindow::setPlanPoints()
{
    return currentPlan->setPlanPoints(currentAgent + 1,
                                      aSelectedArea[currentAgent][0],
                                      aSelectedArea[currentAgent][1]);
}

//Slots for contex menu
//start
void MainWindow::activePassO()
{
    qDebug()<<"Pass Offensive";
    setPlan(PassOffensive);

}

void MainWindow::activePassD()
{
    qDebug()<<"Pass Defensive";
    setPlan(PassDefensive);
}

void MainWindow::activeMoveO()
{
    qDebug()<<"Move Offensive";
    setPlan(MoveOffensive);
}

void MainWindow::activeMoveD()
{
    qDebug()<<"Move Defensive";
    setPlan(MoveDefensive);
}

void MainWindow::activeKickO()
{
    qDebug()<<"Kick Offensive";
    setPlan(KickOffensive);
}

void MainWindow::activeKickD()
{
    qDebug()<<"Kick Defensive";
    setPlan(KickDefensive);
}

void MainWindow::activeChipO()
{
    qDebug()<<"Chip Offensive";
    setPlan(ChipOffensive);
}

void MainWindow::activeChipD()
{
    qDebug()<<"Chip Defensive";
    setPlan(ChipDefensive);
}

void MainWindow::activeMarkO()
{
    qDebug()<<"Mark Offensive";
    setPlan(MarkOffensive);
}

void MainWindow::activeMarkD()
{
    qDebug()<<"Mark Defensive";
    setPlan(MarkDefensive);
}

void MainWindow::activeOneTouch()
{
    qDebug()<<"One Touch";
    setPlan(OneTouch);
}

void MainWindow::activeCatchBall()
{
    qDebug()<<"Catch Ball";
    setPlan(CatchBall);
}

void MainWindow::activeReceivePass()
{
    qDebug()<<"Receive";
    setPlan(ReceivePass);
}

void MainWindow::activeShot()
{
    qDebug()<<"Shot";
    setPlan(Shot);
}

void MainWindow::activeChipToGoal()
{
    qDebug()<<"Chip To Goal";
    setPlan(ChipToGoal);
}
//end

//Contex menu next agent slot
void MainWindow::activePrevAgent()
{
    qDebug()<<"Prev Agent";
    currentAgent--;
    if(currentAgent < 0) currentAgent = 0;
    if(currentPlan->getAgentSkill(currentAgent + 1, currentPriority + 1) == None)
        currentPriority = currentPlan->getPrioritySize(currentAgent + 1);
    if(currentAgent == 0)
        prevAgent->setEnabled(false);
    else
        prevAgent->setEnabled(true);
    if(mode == _1AGENT)
        nextAgent->setEnabled(false);
    else
        nextAgent->setEnabled(true);
    setSkillColors(currentAgent, currentPriority, aOrB);
    blinkCurrentAgent = true;
    showAgentArea(aSelectedArea[currentAgent][aOrB]);

    if(aOrB == 1)
        activeA();
}

//contex menu previous agent slot
void MainWindow::activeNextAgent()
{
    qDebug()<<"Next Agent";
    currentAgent++;
    enableContexAgent();
    if(currentPlan->getAgentSkill(currentAgent + 1, currentPriority + 1) == None)
        currentPriority = currentPlan->getPrioritySize(currentAgent + 1);

    setSkillColors(currentAgent, currentPriority, aOrB);
    blinkCurrentAgent = true;
    showAgentArea(aSelectedArea[currentAgent][aOrB]);

    if(aOrB == 1)
        activeA();
}

//Contex menu previous priority slot
void MainWindow::activePrevPriority()
{
    qDebug()<<"Prev Priority";
    currentPriority--;
    if(currentPriority < 0) currentPriority = 0;
    if(currentPriority == 0)
          prevPriority->setEnabled(false);
    else
          prevPriority->setEnabled(true);
    nextPriority->setEnabled(true);

    setSkillColors(currentAgent, currentPriority, aOrB);
    blinkCurrentAgent = true;
    showAgentArea(aSelectedArea[currentAgent][aOrB]);
}

//Contex menu next priority slot
void MainWindow::activeNextPriority()
{
    qDebug()<<"Next Priority";
    if(currentPlan->getAgentSkill(currentAgent + 1, currentPriority + 1) == None)
        return;
    currentPriority++;
    if(currentPriority > 3) currentPriority = 3;
    if(currentPriority == 3)
          nextPriority->setEnabled(false);
    else
          nextPriority->setEnabled(true);
    prevPriority->setEnabled(true);

    setSkillColors(currentAgent, currentPriority, aOrB);
    blinkCurrentAgent = true;
    showAgentArea(aSelectedArea[currentAgent][aOrB]);
}

//Actives A point for PlayOn
void MainWindow::activeA()
{
    aOrB = 0;
    setSkillColors(currentAgent, currentPriority, aOrB);
    blinkCurrentAgent = true;
    showAgentArea(aSelectedArea[currentAgent][aOrB]);
}

//Actives B point for PlayOn
void MainWindow::activeB()
{
    aOrB = 1;
    setSkillColors(currentAgent, currentPriority, aOrB);
    blinkCurrentAgent = true;
    showAgentArea(aSelectedArea[currentAgent][aOrB]);
}

//Makes exact skil empty in PlayOn
//Actually it will be trigered when right click pressed on skills
void MainWindow::emptySkill(int agent, int priority)
{
    currentPlan->setAgentSkill(agent + 1,
                                priority + 1,
                                aSelectedArea[agent][0],
                                aSelectedArea[priority][1],
                                None);
    if(agent == currentAgent)
    {
        if(currentPlan->getAgentSkill(currentAgent + 2, currentPriority + 2) == None)
            currentPriority = currentPlan->getPrioritySize(currentAgent + 1);
        setSkillColors(currentAgent, currentPriority, aOrB);
    }
    setSkillTexts();
}

//Draws Agents for PlayOn
//Agents ID and Lables
void MainWindow::drawRobot(QPainter &painter, int x, int y, QString label, int agent, bool selected, bool blink)
{
    QPen robotPen[3];
    QBrush robotBrush;
    QRect robotRect;
    QTextOption robotFont;

    if(blink)
        robotPen[1].setColor(QColor(239, 154, 42, 64));
    else
        robotPen[1].setColor(QColor(239, 154, 42));

    robotPen[0].setWidth(2);

    robotBrush.setColor(QColor(57, 103, 128));
    if(selected)
    {
        robotBrush.setStyle(Qt::SolidPattern);
        robotPen[0].setColor(QColor(32, 79, 105));
    }
    else
    {
        robotBrush.setStyle(Qt::Dense3Pattern);
        robotPen[0].setColor(QColor(32, 79, 105, 128));
    }

    robotPen[2].setColor(QColor(Qt::white));

    robotFont.setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    robotRect.setRect(x + 10, y - 20, 16, 16);
    painter.setPen(robotPen[0]);
    painter.setBrush(robotBrush);
    painter.drawEllipse(robotRect);
    painter.setPen(robotPen[2]);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(robotRect, QString::number(agent), robotFont);

    robotRect.setRect(x - 18, y - 18, 36, 36);

    painter.setPen(robotPen[0]);
    painter.setBrush(robotBrush);
    painter.drawEllipse(robotRect);

    painter.setPen(robotPen[1]);
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(robotRect, label, robotFont);

}

//Draws PlayOn robots according to agent size
//It also decide where to draw robots and considers rectabgles layout
void MainWindow::drawRobots(QPainter &painter, int id)
{
    int to;
    switch(mode)
    {
        case _1AGENT:
            to = 1;
        break;
        case _2AGENT:
            to = 2;
        break;
        case _3AGENT:
            to = 3;
        break;
        case _4AGENT:
            to = 4;
        break;
        case _5AGENT:
            to = 5;
        break;
        default:
            to = 0;
        break;
    }

    QList<kkRobot> areaAgents[10];
    kkRobot tempRobot;

    for(int i = 0; i < to; i++)
    {
        for(int j = 0; j < 2; j++)
        {
            if(aSelectedArea[i][j] > 0)
            {
                tempRobot.x = tempRobot.y = -1;
                tempRobot.agent = i + 1;
                if(j == 0)
                {
                    tempRobot.label = "A";
                    tempRobot.aOrB = 0;
                }
                else
                {
                    tempRobot.label = "B";
                    tempRobot.aOrB = 1;
                }
                areaAgents[aSelectedArea[i][j] - 1].append(tempRobot);
            }
        }
    }

    int x, y, w = 39, h = 34, hMargin = 5, vMargin = 6;

    for(int i = 0; i < 10; i++)
    {
        for(int j = 0; j < areaAgents[i].length(); j++)
        {
            if( j == 0)
            {
                x = aRect[i].x() + w/2 + hMargin;
                y = aRect[i].y() + h/2 + vMargin;
            }
            else
            {
                x += w + hMargin;
                if(!aRect[i].contains(x, y))
                {
                    x = aRect[i].x() + w/2 + hMargin;
                    y += h + vMargin;
                }
            }
            tempRobot.x = x;
            tempRobot.y = y;
            tempRobot.label = areaAgents[i].at(j).label;
            tempRobot.agent = areaAgents[i].at(j).agent;
            tempRobot.aOrB = areaAgents[i].at(j).aOrB;
            areaAgents[i].replace(j, tempRobot);
            drawRobot(painter, areaAgents[i].at(j).x,
                      areaAgents[i].at(j).y, areaAgents[i].at(j).label,
                      areaAgents[i].at(j).agent,
                      (areaAgents[i].at(j).agent - 1 == id && areaAgents[i].at(j).aOrB == aOrB)? true : false,
                      (areaAgents[i].at(j).agent - 1 == id && blinkCurrentAgent)? true : false);
        }
    }
}

//Highlights PlayOn region
void MainWindow::showArea(int id, QRect *rect, int size)
{
    QPixmap temp(859, 655);
    temp = *fieldPix;

    QPainter painter(&temp);

    QPen pen1;
    pen1.setColor(QColor(51, 52, 153));
    pen1.setWidth(2);

    QBrush brush1;
    brush1.setColor(QColor(200, 200, 200, 128));
    brush1.setStyle(Qt::SolidPattern);

    QPen pen2;
    pen2.setColor(QColor(255,255,0));
    pen2.setWidth(2);

    QBrush brush2;
    brush2.setColor(Qt::transparent);
    brush2.setStyle(Qt::SolidPattern);

    QTextOption font1;
    font1.setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    painter.setFont( QFont("Arial",36));

    for(int i = 0; i < size; i++)
    {
        painter.setPen(pen1);
        if(id == i + 1)
            painter.setBrush(brush1);
        else
            painter.setBrush(brush2);
        painter.drawRect(rect[i]);
        painter.setPen(pen2);
        painter.drawText(rect[i], QString::number(i + 1),font1);
    }

    if(mode != BALL)
        drawRobots(painter, currentAgent);

    ui->field->setPixmap(temp);

}

//Uses former function for ball
void MainWindow::showBallArea(int id)
{
    showArea(id, bRect, 6);
    setBrief();
}

//Uses former function for agents
void MainWindow::showAgentArea(int id)
{
    showArea(id, aRect, 10);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//Set plan agent size for PLayOn
int MainWindow::getPlanSize(int agentSize)
{
    switch(agentSize)
    {
        default:
        case 1:
            return _1agentPlans.length();
        case 2:
            return _2agentPlans.length();
        case 3:
            return _3agentPlans.length();
        case 4:
            return _4agentPlans.length();
        case 5:
            return _5agentPlans.length();
    }
}

//PlayOn tabwodget, tab change event
void MainWindow::on_tabWidget_currentChanged(int index)
{
    int spinIndex = -1;
    int tCount;
    switch (index) {
        default:
        case 0:
            ui->removeBtn->setEnabled(false);
            mode = BALL;
            aAreaSelected = false;
            showBallArea(bSelectedArea);
            ui->spinBox->setEnabled(false);
            tCount = _1agentPlans.length() + _2agentPlans.length() +
                     _3agentPlans.length() + _4agentPlans.length() +
                     _5agentPlans.length();
            switch(currentPlan->getAgentSize())
            {
                case 1:
                    ui->planSizeLabel->setText("Plan Size: "+QString::number(_1agentPlans.length())+" /"+QString::number(tCount));
                    ui->agentSizeLabel->setText("Agent Size: 1");
                break;
                case 2:
                    ui->planSizeLabel->setText("Plan Size: "+QString::number(_2agentPlans.length())+" /"+QString::number(tCount));
                    ui->agentSizeLabel->setText("Agent Size: 2");
                break;
                case 3:
                    ui->planSizeLabel->setText("Plan Size: "+QString::number(_3agentPlans.length())+" /"+QString::number(tCount));
                    ui->agentSizeLabel->setText("Agent Size: 3");
                break;
                case 4:
                    ui->planSizeLabel->setText("Plan Size: "+QString::number(_4agentPlans.length())+" /"+QString::number(tCount));
                    ui->agentSizeLabel->setText("Agent Size: 4");
                break;
                case 5:
                    ui->planSizeLabel->setText("Plan Size: "+QString::number(_5agentPlans.length())+" /"+QString::number(tCount));
                    ui->agentSizeLabel->setText("Agent Size: 5");
                break;
            }
            ui->commentTextBox->setText(currentPlan->getComment());
        break;
        case 1:
            ui->removeBtn->setEnabled(true);
            mode = _1AGENT;
            bAreaSelected = false;
            showAgentArea(aSelectedArea[currentAgent][aOrB]);
            spinIndex = 0;
            currentPlan->setAgentSize(1);
            ui->spinBox->setEnabled(true);
        break;
        case 2:
            ui->removeBtn->setEnabled(true);
            mode = _2AGENT;
            bAreaSelected = false;
            showAgentArea(aSelectedArea[currentAgent][aOrB]);
            spinIndex = 1;
            currentPlan->setAgentSize(2);
            ui->spinBox->setEnabled(true);
        break;
        case 3:
            ui->removeBtn->setEnabled(true);
            mode = _3AGENT;
            bAreaSelected = false;
            showAgentArea(aSelectedArea[currentAgent][aOrB]);
            spinIndex = 2;
            currentPlan->setAgentSize(3);
            ui->spinBox->setEnabled(true);
        break;
        case 4:
            ui->removeBtn->setEnabled(true);
            mode = _4AGENT;
            bAreaSelected = false;
            showAgentArea(aSelectedArea[currentAgent][aOrB]);
            spinIndex = 3;
            currentPlan->setAgentSize(4);
            ui->spinBox->setEnabled(true);
        break;
        case 5:
            ui->removeBtn->setEnabled(true);
            mode = _5AGENT;
            bAreaSelected = false;
            showAgentArea(aSelectedArea[currentAgent][aOrB]);
            spinIndex = 4;
            currentPlan->setAgentSize(5);
            ui->spinBox->setEnabled(true);
        break;
    }
    if(addBtnMode == 2)
    {
        ui->spinBox->setValue(agentPlanCount[spinIndex]);
        on_spinBox_valueChanged("");
    }
    if(spinIndex != -1)
    {
        tabChanged = true;
        on_spinBox_valueChanged("");
    }
    enableContexAgent();
    setSkillColors(currentAgent, currentPriority, aOrB);
    setSkillTexts();
    lastMode = mode;
    qDebug()<<currentPlan->getPossession();
}

//resets the current plan for PlayOn
void MainWindow::clearPlan(bool resetTab)
{
    bAreaSelected = false;
    bTempArea = bSelectedArea = -1;

    for(int i = 0; i < 5; i++)
        aTempArea[i][0] = aTempArea[i][1] = aSelectedArea[i][0] = aSelectedArea[i][1] = -1;
    aAreaSelected = false;
    currentAgent = 0;
    currentPriority = 0;
    aOrB = 0;

    rightClickEmpty = false;
    ui->ballPossessionCombo->setCurrentIndex(0);
    ui->ballPosCombo->setCurrentIndex(6);
    ui->endPolicyCombo->setCurrentIndex(0);
    ui->endPolicyValCombo->setCurrentIndex(0);
    ui->endPolicyValTextBox->setText("100");
    on_ballPossessionCombo_currentIndexChanged(ui->ballPossessionCombo->currentIndex());
    on_endPolicyValTextBox_editingFinished();
    on_endPolicyValCombo_currentIndexChanged(ui->endPolicyValCombo->currentIndex());
    on_endPolicyCombo_currentIndexChanged(ui->endPolicyCombo->currentIndex());
    if(resetTab)
    {
        ui->tabWidget->setCurrentIndex(0);
        on_tabWidget_currentChanged(0);
    }
}

//Insert PlayOn plans to Struct
void MainWindow::insertPlanToStruct(kkAgentPlan &plan)
{
    for(int i = 0; i < 5; i++)
    {
        plan.agents[i].A = currentPlan->getAgentA(i + 1);
        plan.agents[i].B = currentPlan->getAgentB(i + 1);
        plan.agents[i].pSize = currentPlan->getPrioritySize(i + 1);
        for(int j = 0; j < 4; j++)
            plan.agents[i].skill[j] = currentPlan->getAgentSkill(i + 1, j + 1);
    }
    plan.agentsSize = currentPlan->getAgentSize();
    plan.ball = currentPlan->getBallPos();
    plan.possession = currentPlan->getPossession();
    plan.endMode = currentPlan->getEndPolicyMode();
    plan.endPolicy = currentPlan->getEndPolicyValue();
    plan.planId = currentPlan->getId();
    plan.comment = currentPlan->getComment();
}

//Inserts PlayOn plans to Qlist for handling it easier
void MainWindow::insertPlanToList(QList<kkAgentPlan> &list, int agentSize)
{
    kkAgentPlan tempPlan;

    insertPlanToStruct(tempPlan);

    errorCheck = true;
    ui->statusBar->setStyleSheet("QStatusBar {color: green;}");
    if(currentId + 1 > list.length())
    {
        list.append(tempPlan);
        ui->statusBar->showMessage(QString::number(agentSize)+" Agent"+(agentSize != 1? "s":"")+" plan successfully added!", 3000);
        addBtnMode = 0;
    }
    else
    {
        list.replace(currentId, tempPlan);
        ui->statusBar->showMessage(QString::number(agentSize)+" Agent"+(agentSize != 1? "s":"")+" plan successfully modified! Plan Id: "+QString::number(currentId), 3000);
        addBtnMode = 0;
    }
    addBtnMode = 1;
    ui->addBtn->setText("New");
}

//Add/Apply/New button event
void MainWindow::on_addBtn_clicked()
{
    if(currentVPMode == kkPlayOn)
    {
        if(addBtnMode != 1)
        {
            QString str;
            if(currentPlan->isExecutable(str))
            {
                switch(currentPlan->getAgentSize())
                {
                      default:
                      case 1:
                            insertPlanToList(_1agentPlans, 1);
                      break;
                      case 2:
                            insertPlanToList(_2agentPlans, 2);
                      break;
                      case 3:
                            insertPlanToList(_3agentPlans, 3);
                      break;
                      case 4:
                            insertPlanToList(_4agentPlans, 4);
                      break;
                      case 5:
                            insertPlanToList(_5agentPlans, 5);
                      break;
                }
                ui->addBtn->setText("New");
                agentPlanCount[0] = _1agentPlans.length();
                agentPlanCount[1] = _2agentPlans.length();
                agentPlanCount[2] = _3agentPlans.length();
                agentPlanCount[3] = _4agentPlans.length();
                agentPlanCount[4] = _5agentPlans.length();
            }
            else
            {
                errorCheck = true;
                ui->statusBar->setStyleSheet("QStatusBar {color: red;}");
                ui->statusBar->showMessage(str, 3000);
            }
        }
        else
        {
            agentPlanCount[0] = _1agentPlans.length();
            agentPlanCount[1] = _2agentPlans.length();
            agentPlanCount[2] = _3agentPlans.length();
            agentPlanCount[3] = _4agentPlans.length();
            agentPlanCount[4] = _5agentPlans.length();

            currentPlan->clear();
            clearPlan();
            ui->addBtn->setText("Add");
            addBtnMode = 2;
        }
    }
    else if(currentVPMode == kkPlayOff)
    {
        playOff->apply(POCurrentPlan);
    }

}

//PlayOn ball QComboBox event, (change index event)
void MainWindow::on_ballPosCombo_currentIndexChanged(int index)
{
    if(index != 6)
      bSelectedArea = index + 1;
    else
      bSelectedArea = -1;
    showBallArea(bSelectedArea);
}

//PlayOn Possesion QComboBox Event,  (change index event)
void MainWindow::on_ballPossessionCombo_currentIndexChanged(int index)
{
    currentPlan->setPossession(index);
    setBrief();
}

//PlayOn End Policy QComboBox Ebent, (change index event)
void MainWindow::on_endPolicyCombo_currentIndexChanged(int index)
{
    if(index == 0)
    {
        currentPlan->setEndPolicy(getPolicyByIndex(index+1), ui->endPolicyValTextBox->text().toInt());
        ui->endPolicyValCombo->setVisible(false);
        ui->endPolicyValTextBox->setVisible(true);
        ui->endPolicyValCombo->setEnabled(true);
    }
    else if(index == 2)
    {
        currentPlan->setEndPolicy(getPolicyByIndex(index+1), ui->endPolicyValCombo->currentIndex());
        ui->endPolicyValCombo->setVisible(true);
        ui->endPolicyValTextBox->setVisible(false);
        ui->endPolicyValCombo->setEnabled(false);
    }
    else
    {
        currentPlan->setEndPolicy(getPolicyByIndex(index+1), ui->endPolicyValCombo->currentIndex());
        ui->endPolicyValCombo->setVisible(true);
        ui->endPolicyValTextBox->setVisible(false);
        ui->endPolicyValCombo->setEnabled(true);
    }
    addBtnMode = 0;
    ui->addBtn->setText("Apply");
    setBrief();
}

//PlayOn end policy QComboBox event, (change index event
void MainWindow::on_endPolicyValCombo_currentIndexChanged(int index)
{
    currentPlan->setEndPolicy(getPolicyByIndex(ui->endPolicyCombo->currentIndex()+1), index);
    addBtnMode = 0;
    ui->addBtn->setText("Apply");
    setBrief();
}

//PlayOn end policy QlineEdit event, (edit finished event)
void MainWindow::on_endPolicyValTextBox_editingFinished()
{
    currentPlan->setEndPolicy(getPolicyByIndex(ui->endPolicyCombo->currentIndex()+1),
                              ui->endPolicyValTextBox->text().toInt());
    addBtnMode = 0;
    ui->addBtn->setText("Apply");
    setBrief();
}

//Browse button event
void MainWindow::on_browseBtn_clicked()
{
    if(currentVPMode == kkPlayOn)
    {
        QSettings settings(settingAddress, QSettings::IniFormat);
        QString tempDir = QFileDialog::getOpenFileName(this, tr("Open SQL"),
                                                   settings.value(DEFAULT_DIR_KEY).toString(), tr("SQL Files (*.db3 *.db);;All Files (*.*)"));
        if(tempDir.length() > 2)
        {
            QDir CurrentDir;
            settings.setValue(DEFAULT_DIR_KEY, CurrentDir.absoluteFilePath(tempDir));

            openFileDir = tempDir;
            myPlanSql->changeSQLDir(openFileDir);
            myPlanSql->loadPlan(_1agentPlans, 1);
            myPlanSql->loadPlan(_2agentPlans, 2);
            myPlanSql->loadPlan(_3agentPlans, 3);
            myPlanSql->loadPlan(_4agentPlans, 4);
            myPlanSql->loadPlan(_5agentPlans, 5);

            agentPlanCount[0] = _1agentPlans.length();
            agentPlanCount[1] = _2agentPlans.length();
            agentPlanCount[2] = _3agentPlans.length();
            agentPlanCount[3] = _4agentPlans.length();
            agentPlanCount[4] = _5agentPlans.length();
            /*for(int i = 0; i < 5; i++)
                if(agentPlanCount[i] == -1)
                    agentPlanCount[i] = 0;*/

            currentPlan->clear(1);
            clearPlan();
            mode = _1AGENT;
            ui->spinBox->setValue(0);
            on_spinBox_valueChanged("");
            on_tabWidget_currentChanged(0);

            //sets plan directory in window title
            setWindowTitle("Visual Planner - "+openFileDir);
        }
    }
    else if(currentVPMode == kkPlayOff)
    {
        QSettings settings(settingAddress, QSettings::IniFormat);
        QString tempDir = QFileDialog::getOpenFileName(this, tr("Open SQL"),
                                                   settings.value(DEFAULT_DIR_KEY).toString(), tr("SQL Files (*.db3 *.db);;All Files (*.*)"));
        if(tempDir.length() > 2)
        {
            QDir CurrentDir;
            settings.setValue(DEFAULT_DIR_KEY, CurrentDir.absoluteFilePath(tempDir));

            POopenFileDir = tempDir;

            playOff->cleanPlans();
            ui->spinBox->setEnabled(true);
            int tempCnt = playOff->loadPlan(POopenFileDir);
            if(tempCnt > 0)
                playOff->loadPOPlan(0);
            ui->spinBox->setMaximum(tempCnt-1);
            POCurrentPlan = 0;
            on_spinBox_valueChanged("");

            //sets plan directory in window title
            setWindowTitle("Visual Planner - "+POopenFileDir);
        }
    }
}

//save button event
void MainWindow::on_saveBtn_clicked()
{
    if(currentVPMode == kkPlayOn)
    {
        QMessageBox::StandardButton reply;
        if(openFileDir.length() < 2)
        {
            reply = QMessageBox::question(this, "Save SQL", "Do you want to save?",
                                          QMessageBox::Yes|QMessageBox::Cancel);
            if (reply == QMessageBox::Cancel)
                return;
        }
        else
        {
            reply = QMessageBox::question(this, "Save SQL", "Do you want to save?",
                                          QMessageBox::Save|QMessageBox::Yes|QMessageBox::Cancel);
            if (reply == QMessageBox::Cancel)
                return;
        }

        if(openFileDir.length() < 2 || reply == QMessageBox::Save)
        {
            QString tempDir;
            tempDir = QFileDialog::getSaveFileName(this, tr("Save plan file"),
                                                   QDir::currentPath(), tr("SQL Files (*.db3 *.db);;All Files (*.*)"));
            if(tempDir.length() > 2)
                saveFileDir = tempDir;
            else
                return;
        }
        else
            saveFileDir = openFileDir;


        myPlanSql->addPlan(_1agentPlans, 1);
        myPlanSql->addPlan(_2agentPlans, 2);
        myPlanSql->addPlan(_3agentPlans, 3);
        myPlanSql->addPlan(_4agentPlans, 4);
        myPlanSql->addPlan(_5agentPlans, 5);

        myPlanSql->changeSQLDir(saveFileDir);
        myPlanSql->savePlan();

        //sets plan directory in window title
        setWindowTitle("Visual Planner - "+saveFileDir);

    }
    else if(currentVPMode == kkPlayOff)
    {
        QMessageBox::StandardButton reply;
        if(POopenFileDir.length() < 2)
        {
            reply = QMessageBox::question(this, "Save SQL", "Do you want to save?",
                                          QMessageBox::Yes|QMessageBox::Cancel);
            if (reply == QMessageBox::Cancel)
                return;
        }
        else
        {
            reply = QMessageBox::question(this, "Save SQL", "Do you want to save?",
                                          QMessageBox::Save|QMessageBox::Yes|QMessageBox::Cancel);
            if (reply == QMessageBox::Cancel)
                return;
        }

        if(reply == QMessageBox::Yes)
        {
            //removes the plan file with same name
            QFile::remove(POopenFileDir);
        }
        if(POopenFileDir.length() < 2 || reply == QMessageBox::Save)
        {
            QString tempDir;
            tempDir = QFileDialog::getSaveFileName(this, tr("Save plan file"),
                                                   QDir::currentPath(), tr("SQL Files (*.db3 *.db);;All Files (*.*)"));
            if(tempDir.length() > 2)
                POsaveFileDir = tempDir;
            else
                return;
        }
        else
            POsaveFileDir = POopenFileDir;

        playOff->savePlan(POsaveFileDir);

        //sets plan directory in window title
        setWindowTitle("Visual Planner - "+POsaveFileDir);
    }
}

//Reset button event
void MainWindow::on_resetBtn_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Reset Plan", "Are you sure?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        if(currentVPMode == kkPlayOn)
        {
            currentPlan->clear();
            clearPlan();
        }
        else if(currentVPMode == kkPlayOff)
        {
            playOff->reset();
        }
    }
    /*else
    {
        qDebug() << "Yes was *not* clicked";
    }*/
}

//loads PlayOn plans from its sql class
void MainWindow::loadPlan(kkAgentPlan plan)
{
    if(currentVPMode == kkPlayOn)
    {
        currentPlan->clear();
        bSelectedArea = plan.ball;
        currentPlan->setAgentSize(plan.agentsSize);
        currentPlan->setBallPos(plan.ball);
        currentPlan->setEndPolicy(plan.endMode, plan.endPolicy);
        ui->endPolicyCombo->setCurrentIndex(getIntByEnum(plan.endMode)-1);
        switch(plan.endMode)
        {
            case Cycle :
                ui->endPolicyValTextBox->setText(QString::number(plan.endPolicy));
            break;
            case AllAgents:
            case ExactDisturb:
            case ExactAgent:
                ui->endPolicyValCombo->setCurrentIndex(plan.endPolicy);
            break;
        }
        currentPlan->setPlanId(plan.planId);
        currentPlan->setPossession(plan.possession);
        ui->ballPossessionCombo->setCurrentIndex(plan.possession);
        currentPlan->setComment(plan.comment);
        for(int i = 0; i < 5; i++)
        {
            aSelectedArea[i][0] = plan.agents[i].A;
            aSelectedArea[i][1] = plan.agents[i].B;
            for(int j = 0; j < 4; j++)
            {
                if(plan.agents[i].skill[j] != None)
                {
                    currentPlan->setAgentSkill(i + 1, j + 1,
                                               plan.agents[i].A,
                                               plan.agents[i].B,
                                               plan.agents[i].skill[j]);
                }
            }
        }
        setSkillColors(0, 0, 0);
        setSkillTexts();
        showAgentArea(aSelectedArea[0][0]);
    }
    else if(currentVPMode == kkPlayOff)
    {

    }
}

//up
void MainWindow::loadPlan(int planId, PlannerMode tMode)
{
    switch(tMode)
    {
        case _1AGENT:
            if(planId >= _1agentPlans.length())
                return;
            loadPlan(_1agentPlans.at(planId));
        break;
        case _2AGENT:
            if(planId >= _2agentPlans.length())
                return;
            loadPlan(_2agentPlans.at(planId));
        break;
        case _3AGENT:
            if(planId >= _3agentPlans.length())
                return;
            loadPlan(_3agentPlans.at(planId));
        break;
        case _4AGENT:
            if(planId >= _4agentPlans.length())
                return;
            loadPlan(_4agentPlans.at(planId));
        break;
        case _5AGENT:
            if(planId >= _5agentPlans.length())
                return;
            loadPlan(_5agentPlans.at(planId));
        break;
    }
}

//applies changes for PlayOn
bool MainWindow::applyEdit()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Apply Changes", "Are you sure?",
                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel)
        return false;
    else if(reply == QMessageBox::No)
    {
        return true;
    }
    else if(reply == QMessageBox::Yes)
    {
        addBtnMode = 0;
        on_addBtn_clicked();
        return true;
    }
}

//Plan explorer event
void MainWindow::on_spinBox_valueChanged(const QString &arg1)
{
    int tempSpin = ui->spinBox->value();
    if(currentVPMode == kkPlayOn)
    {

        int spinIndex;

        switch(mode)
        {
            default:
            case _1AGENT:
                if(tempSpin > agentPlanCount[0])
                    tempSpin = agentPlanCount[0];
            break;
            case _2AGENT:
                if(tempSpin > agentPlanCount[1])
                    tempSpin = agentPlanCount[1];
            break;
            case _3AGENT:
                if(tempSpin > agentPlanCount[2])
                    tempSpin = agentPlanCount[2];
            break;
            case _4AGENT:
                if(tempSpin > agentPlanCount[3])
                    tempSpin = agentPlanCount[3];
            break;
            case _5AGENT:
                if(tempSpin > agentPlanCount[4])
                    tempSpin = agentPlanCount[4];
            break;
        }

        if(tempSpin == getPlanSize(getIntByEnum(mode)) || (unsavedPlanFlag && tabChanged))
        {
            unsavedPlanFlag = true;
            tempSpin = 10000;
        }
        else
            unsavedPlanFlag = false;

        if(unsavedPlanFlag)
        {
            insertPlanToStruct(unsavedPlan);
        }

        if(tabChanged && !unsavedPlanFlag)
        {
            tempSpin = 0;
            ui->spinBox->setValue(0);
        }

        switch(mode)
        {
            default:
            case _1AGENT:
                if(tempSpin > agentPlanCount[0])
                {
                    tempSpin = agentPlanCount[0];
                    ui->spinBox->setValue(tempSpin);
                }
                spinIndex = 0;
            break;
            case _2AGENT:
                if(tempSpin > agentPlanCount[1])
                {
                    tempSpin = agentPlanCount[1];
                    ui->spinBox->setValue(tempSpin);
                }
                spinIndex = 1;
            break;
            case _3AGENT:
                if(tempSpin > agentPlanCount[2])
                {
                    tempSpin = agentPlanCount[2];
                    ui->spinBox->setValue(tempSpin);
                }
                spinIndex = 2;
            break;
            case _4AGENT:
                if(tempSpin > agentPlanCount[3])
                {
                    tempSpin = agentPlanCount[3];
                    ui->spinBox->setValue(tempSpin);
                }
                spinIndex = 3;
            break;
            case _5AGENT:
                if(tempSpin > agentPlanCount[4])
                {
                    tempSpin = agentPlanCount[4];
                    ui->spinBox->setValue(tempSpin);
                }
                spinIndex = 4;
            break;
        }

        if(unsavedPlanFlag)
            loadPlan(unsavedPlan);
        else
            loadPlan(tempSpin, mode);

        lastPlan = tempSpin;
        currentId = tempSpin;
        tabChanged = false;
    }
    else if(currentVPMode == kkPlayOff)
    {
        POCurrentPlan = tempSpin;
        playOff->loadPOPlan(tempSpin);
        if(POCurrentPlan < playOff->getPlanSize() - 1 && playOff->getPlanSize() > 0)
            ui->removeBtn->setDisabled(false);
        else
            ui->removeBtn->setDisabled(true);
    }
}

//Commennt event for PlayOn
void MainWindow::on_commentTextBox_textChanged()
{
    /*QString tempText = ui->commentTextBox->toPlainText();
    QStringList tempTextList = tempText.split("\n");
    tempText = tempTextList.join("*spc*");*/
    currentPlan->setComment(ui->commentTextBox->toPlainText());
}

//Removes plan
void MainWindow::on_removeBtn_clicked()
{
    if(currentVPMode == kkPlayOn)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Remove Plan", "Are you sure?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return;

        switch(mode)
        {
            case _1AGENT:
                if(currentId < getPlanSize(1))
                {
                    _1agentPlans.removeAt(currentId);
                    agentPlanCount[0] = _1agentPlans.length();
                    ui->spinBox->setValue(currentId);
                    on_spinBox_valueChanged("");
                    return;
                }
            break;
            case _2AGENT:
                if(currentId < getPlanSize(2))
                {
                    _2agentPlans.removeAt(currentId);
                    agentPlanCount[1] = _2agentPlans.length();
                    ui->spinBox->setValue(currentId);
                    on_spinBox_valueChanged("");
                    return;
                }
            break;
            case _3AGENT:
                if(currentId < getPlanSize(3))
                {
                    _3agentPlans.removeAt(currentId);
                    agentPlanCount[2] = _3agentPlans.length();
                    ui->spinBox->setValue(currentId);
                    on_spinBox_valueChanged("");
                    return;
                }
            break;
            case _4AGENT:
                if(currentId < getPlanSize(4))
                {
                    _4agentPlans.removeAt(currentId);
                    agentPlanCount[3] = _4agentPlans.length();
                    ui->spinBox->setValue(currentId);
                    on_spinBox_valueChanged("");
                    return;
                }
            break;
            case _5AGENT:
                if(currentId < getPlanSize(5))
                {
                    _5agentPlans.removeAt(currentId);
                    agentPlanCount[4] = _5agentPlans.length();
                    ui->spinBox->setValue(currentId);
                    on_spinBox_valueChanged("");
                    return;
                }
            break;
        }

        currentPlan->clear(currentPlan->getAgentSize());
        clearPlan(false);
    }
    else if(currentVPMode == kkPlayOff)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Remove Plan", "Are you sure?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return;

        playOff->removePlan(POCurrentPlan);
        ui->spinBox->setValue(0);
        POCurrentPlan = 0;
        on_spinBox_valueChanged("");
    }
}

//Sets yellow Brief QLable for PlayOn
//Since all information isn't in same page, it helps users to realyze plan easier
void MainWindow::setBrief()
{
    QString str;
    str += "Ball's ";

    switch(currentPlan->getPossession())
    {
        case 0:
            str += "ours";
        break;
        case 1:
            str += "theirs";
        break;
        case 2:
            str += "no ones";
        break;
    }

    str += " and in R";

    switch(bSelectedArea)
    {
        case 1:
            str += "1";
        break;
        case 2:
            str += "2/3/4";
        break;
        case 3:
            str += "5";
        break;
        case 4:
            str += "6";
        break;
        case 5:
            str += "7";
        break;
        case 6:
            str += "8/9/10";
        break;
        default:
            str += "-NA-";
        break;
    }

    str += ". Plan ends ";

    switch(currentPlan->getEndPolicyMode())
    {
        case Cycle :
            str += "in "+QString::number(currentPlan->getEndPolicyValue())+" cycles";
        break;
        case ExactAgent:
            str += "with A"+QString::number(currentPlan->getEndPolicyValue()+1)+"'s Task";
        break;
        case AllAgents:
            str += "With All Agents' tasks";
        break;
        case ExactDisturb:
            str += "if A"+QString::number(currentPlan->getEndPolicyValue()+1)+"'s Task disturbs";
        break;
    }

    str += ".";

    ui->ballLabel->setText(str);
}

//Menu event for PlayOn
//Shortcut is Ctrl+N
void MainWindow::on_actionPlayOn_triggered()
{
    ui->menuBar->actions().at(0)->menu()->actions().at(0)->setChecked(true);
    ui->menuBar->actions().at(0)->menu()->actions().at(1)->setChecked(false);
    currentVPMode = kkPlayOn;
    myPlanSql->enableSQL();
    playOff->disableSQL();
    initVPMode();
    ui->newBtn->setVisible(false);
}

//Menu event for PlayOff
//Shortcut is Ctrl+M
void MainWindow::on_actionPlayOff_triggered()
{
    ui->menuBar->actions().at(0)->menu()->actions().at(0)->setChecked(false);
    ui->menuBar->actions().at(0)->menu()->actions().at(1)->setChecked(true);
    currentVPMode = kkPlayOff;
    myPlanSql->disableSQL();
    playOff->enableSQL();
    initVPMode();
    ui->newBtn->setVisible(true);
    if(POSpinInit)
    {
        ui->spinBox->setMaximum(0);
        POSpinInit = false;
    }
}

//Initilize Visual Planner modes
void MainWindow::initVPMode()
{
    if(currentVPMode == kkPlayOn)
    {
        ui->tabWidget->setVisible(true);
        ui->ballLabel->setVisible(true);
        ui->POtabWidget->setVisible(false);
        showArea(-1, bRect, 6);
    }
    else if(currentVPMode == kkPlayOff)
    {
        ui->tabWidget->setVisible(false);
        ui->ballLabel->setVisible(false);
        ui->POtabWidget->setVisible(true);
        playOff->draw();
    }
}

void MainWindow::on_comboBox_2_currentIndexChanged(int index)
{
    playOff->setAgentSize(index + 1);
    qDebug()<<index<<playOff->getCurrentAgent();
    if(playOff->getCurrentAgent() > index)
    {
        playOff->setCurrentAgent(index);
    }
    playOff->POSetSelectedDisplayLabel(playOff->getDisplayMode());
    playOff->POSetSelectedCurrentAgentLabel(playOff->getCurrentAgent());
}

//PlayOff geometry QLineEdit events
//start
void MainWindow::on_POBtn_pressed()
{
    playOff->setShowAllFlag(true);
}

void MainWindow::on_POBtn_released()
{
    playOff->setShowAllFlag(false);
}

void MainWindow::on_POTBPosX_returnPressed()
{
    playOff->setGeomX(ui->POTBPosX->text().toInt());
}

void MainWindow::on_POTBPosY_returnPressed()
{
    playOff->setGeomY(ui->POTBPosY->text().toInt());
}

void MainWindow::on_POTBPosAng_returnPressed()
{
    int tempAng = ui->POTBPosAng->text().toInt();
    if(tempAng > 180) tempAng = 180;
    if(tempAng < -180) tempAng = -180;
    ui->POTBPosAng->setText(QString::number(tempAng));
    playOff->setGeomAngle(tempAng);
}

void MainWindow::on_POTBPosTol_returnPressed()
{
    playOff->setGeomTolerance(ui->POTBPosTol->text().toInt());
}
//end
//Creates contex menu for PlayOff
void MainWindow::POCreateActions()
{
    POPass = new QAction(tr("&Pass"), this);
    POPass->setStatusTip(tr("Select the target agent"));
    connect(POPass, SIGNAL(triggered()), this, SLOT(POActivePass()));

    POReceivePass = new QAction(tr("&Receive Pass"), this);
    POReceivePass->setStatusTip(tr("Select receive pass agent"));
    connect(POReceivePass, SIGNAL(triggered()),this,SLOT(POActiveReceivePass()));

    POShotToGoal = new QAction(tr("&Shot to Goal"), this);
    POShotToGoal->setStatusTip(tr("Select shoter agent"));
    connect(POShotToGoal, SIGNAL(triggered()),this,SLOT(POActiveShotToGoal()));

    POChipToGoal = new QAction(tr("&Chip to Goal"), this);
    POChipToGoal->setStatusTip(tr("Select chipper agent"));
    connect(POChipToGoal, SIGNAL(triggered()),this,SLOT(POActiveChipToGoal()));

    POOneTouch = new QAction(tr("&One Touch"), this);
    POOneTouch->setStatusTip(tr("Select one toucher"));
    connect(POOneTouch, SIGNAL(triggered()),this,SLOT(POActiveOneTouch()));

    POMove = new QAction(tr("&Move"), this);
    POMove->setStatusTip(tr("Move"));
    connect(POMove, SIGNAL(triggered()),this,SLOT(POActiveMove()));
}

//PlayOff contex menu slots
//start
void MainWindow::POActivePass()
{
    qDebug()<<"Active Pass";
    playOff->setSkill(PassSkill);
}

void MainWindow::POActiveReceivePass()
{
    qDebug()<<"Active Receive Pass";
    playOff->setSkill(ReceivePassSkill);
}

void MainWindow::POActiveShotToGoal()
{
    qDebug()<<"Active Shot to Goal";
    playOff->setSkill(ShotToGoalSkill);
}

void MainWindow::POActiveChipToGoal()
{
    qDebug()<<"Active Chip to Goal";
    playOff->setSkill(ChipToGoalSkill);
}

void MainWindow::POActiveOneTouch()
{
    qDebug()<<"Active One Touch";
    playOff->setSkill(OneTouchSkill);
}

void MainWindow::POActiveMove()
{
    qDebug()<<"Active Move";
    playOff->setSkill(MoveSkill);
}
//end

//New button fot PlayOff
void MainWindow::on_newBtn_clicked()
{
    ui->spinBox->setMaximum(playOff->getPlanSize());
    ui->spinBox->setValue(playOff->getPlanSize());
    ui->spinBox->setEnabled(true);
    on_spinBox_valueChanged("");
    playOff->clean();
}