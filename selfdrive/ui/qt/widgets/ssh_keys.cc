#include "selfdrive/ui/qt/widgets/ssh_keys.h"

#include "selfdrive/common/params.h"
#include "selfdrive/ui/qt/api.h"
#include "selfdrive/ui/qt/widgets/input.h"

SshControl::SshControl() : ButtonControl("SSH公鑰", "", "警告：這將授予對GitHub設定中所有公鑰的SSH存取權。切勿輸入您自己以外的GitHub使用者名稱。COMMA AI員工絕對不會要求您新增他們的GitHub使用者名稱") {
  username_label.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  username_label.setStyleSheet("color: #aaaaaa");
  hlayout->insertWidget(1, &username_label);

  QObject::connect(this, &ButtonControl::clicked, [=]() {
    if (text() == "ADD") {
      QString username = InputDialog::getText("輸入你的GitHub使用者名稱", this);
      if (username.length() > 0) {
        setText("載入中");
        setEnabled(false);
        getUserKeys(username);
      }
    } else {
      params.remove("GithubUsername");
      params.remove("GithubSshKeys");
      refresh();
    }
  });

  refresh();
}

void SshControl::refresh() {
  QString param = QString::fromStdString(params.get("GithubSshKeys"));
  if (param.length()) {
    username_label.setText(QString::fromStdString(params.get("GithubUsername")));
    setText("移除");
  } else {
    username_label.setText("");
    setText("新增");
  }
  setEnabled(true);
}

void SshControl::getUserKeys(const QString &username) {
  HttpRequest *request = new HttpRequest(this, false);
  QObject::connect(request, &HttpRequest::requestDone, [=](const QString &resp, bool success) {
    if (success) {
      if (!resp.isEmpty()) {
        params.put("GithubUsername", username.toStdString());
        params.put("GithubSshKeys", resp.toStdString());
      } else {
        ConfirmationDialog::alert(QString("GitHub使用者名稱: '%1' 不存在SSH Keys").arg(username), this);
      }
    } else {
      if (request->timeout()) {
        ConfirmationDialog::alert("連線逾時", this);
      } else {
        ConfirmationDialog::alert(QString("GitHub使用者名稱: '%1' 不存在").arg(username), this);
      }
    }

    refresh();
    request->deleteLater();
  });

  request->sendRequest("https://github.com/" + username + ".keys");
}
