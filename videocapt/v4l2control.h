#ifndef V4L2CONTROL_H
#define V4L2CONTROL_H

#include <QObject>
#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <libv4l2.h>
#include <QEvent>

class V4L2Control : public QWidget
{
    Q_OBJECT
public slots:
    void updateHardware();
    virtual void updateStatus();
    virtual void resetToDefault();
    virtual void setValue(int val) = 0;

public:
    virtual int getValue() = 0;

protected:
    V4L2Control(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent);
    int fd;
    int cid;
    int default_value;
    char name[33];
    QHBoxLayout layout;
};

class V4L2IntegerControl : public V4L2Control
{
    Q_OBJECT
public:
    V4L2IntegerControl(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent);

public slots:
    void setValue(int val);

public:
    int getValue();

private slots:
    void SetValueFromSlider(void);
    void SetValueFromText(void);

private:
    int minimum;
    int maximum;
    int step;
    QSlider *sl;
    QLineEdit *le;
};

class V4L2BooleanControl : public V4L2Control
{
    Q_OBJECT
public:
    V4L2BooleanControl(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent);

public slots:
    void setValue(int val);

public:
    int getValue();

private:
    QCheckBox *cb;
};

class V4L2MenuControl : public V4L2Control
{
    Q_OBJECT
public:
    V4L2MenuControl(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent);

public slots:
    void setValue(int val);

public:
    int getValue();

private:
    QComboBox *cb;
    short int indexValueArray[256];

private slots:
    void menuActivated(int val);
};

class V4L2ButtonControl : public V4L2Control
{
    Q_OBJECT
public slots:
    void updateStatus();
    void resetToDefault();

public:
    V4L2ButtonControl(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent);

public slots:
    void setValue(int) {};
    int getValue() { return 0; };
};

#endif // V4L2CONTROL_H
