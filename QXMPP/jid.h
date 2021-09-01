#ifndef JID_H
#define JID_H

#include <QString>

namespace QXMPP {

  enum class ShowStatus {
    online,
    dnd,
    xa,
    away,
    chat,
    unknown
  };

  enum class TLSType {
    AUTO,
    TLS_ONLY,
    NO_TLS
  };

  enum class SASLType {
    AUTO,
    Plain,
    DigestMD5,
    SCRAM_SHA1
  };

  struct Jid {
    Jid() {}
    Jid(const Jid &other) {
      this->sasl = other.sasl;
      this->tls = other.tls;
      this->show_status = other.show_status;
      this->user = other.user;
      this->host = other.host;
      this->password = other.password;
      this->resource = other.resource;
      this->msg_status = other.msg_status;
      this->user_info = other.user_info;
      this->user_group = other.user_info;
      this->port = other.port;
      this->priority = other.priority;
      this->keep_alive = other.keep_alive;
    }
    Jid(QString full_jid) {
      int pos_srv = full_jid.indexOf("@");
      int pos_res = full_jid.indexOf('/');
      if (-1 != pos_srv) {
        user = full_jid.mid(0, pos_srv);
        if (-1 != pos_res) {
          host = full_jid.mid(pos_srv + 1, pos_res - pos_srv -1);
          resource = full_jid.mid(pos_res + 1);
        }
        else {
          host = full_jid.mid(pos_srv + 1);
          resource.clear();
        }
      }
    }

    void operator=(Jid &other) {
      this->sasl = other.sasl;
      this->tls = other.tls;
      this->show_status = other.show_status;
      this->user = other.user;
      this->host = other.host;
      this->password = other.password;
      this->resource = other.resource;
      this->msg_status = other.msg_status;
      this->user_info = other.user_info;
      this->user_group = other.user_info;
      this->port = other.port;
      this->priority = other.priority;
      this->keep_alive = other.keep_alive;
    }

    QString getJid() {
      return QString("%1@%2").arg(user).arg(host);
    }

    static QString getJid(QString full_jid) {
      int pos = full_jid.indexOf('/');
      if (-1 == pos) return full_jid;
      else {
        return full_jid.mid(0, pos);
      }
    }

    QString getFullJid() {
      if (resource.isEmpty()) return getJid();
      else return QString("%1@%2/%3").arg(user).arg(host).arg(resource);
    }

    void setShowStatus(const QString &str_show_status) {
      if (str_show_status == "online") this->show_status = ShowStatus::online;
      else if (str_show_status == "dnd") this->show_status = ShowStatus::dnd;
      else if (str_show_status == "xa") this->show_status = ShowStatus::xa;
      else if (str_show_status == "away") this->show_status = ShowStatus::away;
      else if (str_show_status == "chat") this->show_status = ShowStatus::chat;
      else if (str_show_status == "unknown") this->show_status = ShowStatus::unknown;
      else this->show_status = ShowStatus::unknown;
    }

    QString getShowStatus() {
      switch(this->show_status) {
        case ShowStatus::online : return "online";
        case ShowStatus::dnd : return "dnd";
        case ShowStatus::xa : return "xa";
        case ShowStatus::away : return "away";
        case ShowStatus::chat : return "chat";
        case ShowStatus::unknown : return "unknown";
      }
      return "unknown";
    }

    QXMPP::SASLType sasl = QXMPP::SASLType::AUTO;
    QXMPP::TLSType  tls = QXMPP::TLSType::AUTO;
    QXMPP::ShowStatus show_status = QXMPP::ShowStatus::online;
    QString user{};
    QString host{};
    QString password{};
    QString resource = "QXMPP_LITE_JABBER";
    QString msg_status = "I USED QXMPP LITE";
    QString user_info{};
    QString user_group{};
    qint16  port = 5222;
    int priority = 5;
    bool keep_alive = true;
  };

}

#endif // JID_H
