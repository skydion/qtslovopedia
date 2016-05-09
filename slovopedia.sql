drop database if exists slovopedia;

create database slovopedia DEFAULT CHARACTER SET = UTF8;
use slovopedia;

drop table if exists works;

create table works
(
  _id int unsigned not null auto_increment primary key,
  index _id_ind(_id),

  # слово
  _word tinytext not null,

  # редаговане пояснення до слова
  _question text not null,

  # оригінальний текст з словників
  _text text not null,

  # використано запитання до слова чи ні, краще робити це для запитання
  # бо їх може бути будти декілька для одного слова, що "збільшує" кількість
  # слів для кросвордів
  _used tinyint not null default 0,

  # мова
  # 0 - українська
  # 1 - російська
  # 2 - англійська
  _language tinyint not null default 0,

  # номер словника, може для тематичних кросводів
  _vocabulary tinyint unsigned not null default 0
) type=innodb;
