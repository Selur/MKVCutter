#include "mkvcutter.h"

#include <QtGui>
#include <QApplication>
#include <QCommandLineParser>
#include <iostream>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  QCoreApplication::setApplicationName("MkvCutter");
  QCoreApplication::setApplicationVersion(QString::fromLocal8Bit(BUILDDATE));

  QCommandLineParser parser;
  parser.setApplicationDescription("MkvCutter - lossless H.264 MKV cutter");
  parser.addHelpOption();
  parser.addVersionOption();

  // --clinput=<command>   repeatable, processed in order after the UI is up
  // current commands:
  //   open:<path>           load a .mkv file (same as File->Open + Select)
  //   output:<path>         set the output file
  //   temp:<path>           set the temp folder
  //   keepIntermediate:on   turn the keepIntermediate checkbox on
  //   keepIntermediate:off  turn it off
  //   cutlist:<path>        load a .cut file into the cut view
  //   commit                commit the cut view (same as the Commit button)
  //   next                  press the "Next" / commit button
  //   scanorder:<auto|bff|tff>  override the scan-order
  //   quit                  close the application
  //
  // Note: every command is processed before the event loop starts, so the pipeline that
  // 'open' kicks off is still running. 'cutlist', 'commit' and 'next' therefore defer
  // themselves to the matching milestone. 'open' resets the GUI, so it has to come
  // BEFORE 'output' and 'temp' -- otherwise reset() wipes them again.
  // A full unattended run looks like this:
  //   --clinput=open:in.mkv --clinput=output:out.mkv --clinput=temp:C:\tmp
  //   --clinput=cutlist:cut-list.cut --clinput=commit --clinput=next
  QCommandLineOption clinput("clinput",
      "Control the GUI after startup. Repeatable, processed in order.",
      "command");
  parser.addOption(clinput);

  parser.process(a);

  MkvCutter w;
  w.show();

  const QStringList commands = parser.values(clinput);
  for (int i = 0; i < commands.size(); ++i) {
    const QString cmd = commands.at(i);
    const int sep = cmd.indexOf(':');
    const QString name = (sep < 0) ? cmd : cmd.left(sep);
    const QString arg  = (sep < 0) ? QString() : cmd.mid(sep + 1);
    std::cerr << "clinput[" << i << "]: " << qPrintable(cmd) << std::endl;
    if (name == "open") {
      if (!arg.isEmpty()) {
        w.cliOpen(arg);
      }
    } else if (name == "output") {
      if (!arg.isEmpty()) {
        w.cliSetOutput(arg);
      }
    } else if (name == "temp") {
      if (!arg.isEmpty()) {
        w.cliSetTemp(arg);
      }
    } else if (name == "keepIntermediate") {
      w.cliSetKeepIntermediate(arg == "on" || arg == "1" || arg == "true");
    } else if (name == "cutlist") {
      if (!arg.isEmpty()) {
        w.cliLoadCutList(arg);
      }
    } else if (name == "commit") {
      w.cliCommit();
    } else if (name == "next") {
      w.cliNext();
    } else if (name == "scanorder") {
      if (!arg.isEmpty()) {
        w.cliSetScanOrder(arg);
      }
    } else if (name == "quit") {
      QTimer::singleShot(0, &a, &QCoreApplication::quit);
    } else {
      std::cerr << "clinput: unknown command '" << qPrintable(name) << "'" << std::endl;
    }
  }

  return a.exec();
}
