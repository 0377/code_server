use account;
DROP TABLE IF EXISTS `t_channel_account`;
CREATE TABLE `t_channel_account` (
  `uid` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `username` varchar(60) NOT NULL DEFAULT '' COMMENT '�����ʺ�',
  `password` varchar(100) NOT NULL DEFAULT '' COMMENT '��������',
  `Payment` varchar(50) NOT NULL DEFAULT '' COMMENT '��ע',
  `chargeType` tinyint(1) NOT NULL DEFAULT '0' COMMENT '�Ʒ����� 1 ��װ�Ʒ� 2 ���',
  `price` double(9,2) NOT NULL DEFAULT '0.00' COMMENT '��װ�������õ��ۼƷ�',
  `CommissionRate` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '�����',
  `email` varchar(100) NOT NULL DEFAULT '' COMMENT '����',
  `Grade` tinyint(4) NOT NULL DEFAULT '0' COMMENT '�ȼ�',
  `status` tinyint(4) NOT NULL DEFAULT '1' COMMENT '1���� 0 ͣ��',
  `father_id` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '��ID',
  `login_times` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '��½ʱ��',
  `Rate` tinyint(4) NOT NULL DEFAULT '0' COMMENT '�����ٷ��� 80 ��ʾ 80%������',
  `old_status` tinyint(4) NOT NULL DEFAULT '0' COMMENT '�Ƿ�������',
  `Must` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'ǰ���ٰ�װ�ٷְٳ���',
  `qq` bigint(20) NOT NULL DEFAULT '0' COMMENT '��ϵQQ',
  `phone` bigint(20) NOT NULL COMMENT '��ϵ�绰',
  `name` varchar(50) NOT NULL COMMENT '����',
  `BankMsg` varchar(100) NOT NULL COMMENT '֧����Ϣ',
  `BankAccount` varchar(200) NOT NULL COMMENT '֧���˺�',
  `down` text NOT NULL,
  PRIMARY KEY (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=2 COMMENT '�����˻���';

INSERT INTO `t_channel_account` (`uid`, `username`, `password`,`father_id`, `Payment`, `price`, `email`, `Grade`, `status`,  `login_times`, `Rate`, `old_status`, `Must`, `qq`, `phone`, `name`, `BankMsg`, `BankAccount`, `down`) VALUES
(1, 'admin', '5425621d9ce4961a91a42d12004b60ca',0, '', 0.00, '123456@qq.com', 1, 1, 1512099418, 0, 0, 0, 123456, 18888888888, 'admin', '123', '888888', '');

DROP TABLE IF EXISTS `t_default`;
CREATE TABLE IF NOT EXISTS `t_default` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `Rate` tinyint(4) NOT NULL DEFAULT '0' COMMENT 'Ĭ�ϳ�����',
  `Must` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Ĭ��ǰ���س�',
  `price` double(5,2) NOT NULL DEFAULT '0.00' COMMENT 'Ĭ�ϰ�װ����',
  `CommissionRate` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Ĭ�������',
  `cache_times` int(10) unsigned NOT NULL DEFAULT '0',
  `cache_daytimes` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=0 COMMENT 'Ĭ�����ñ�' ;


INSERT INTO `t_default` (`id`, `Rate`, `Must`, `price`, `cache_times`, `cache_daytimes`, `CommissionRate`) VALUES
(1, 70, 100, 0.28, 1416300241, 1416300301, 30);

DROP TABLE IF EXISTS `t_channelDetailed_201712`;
CREATE TABLE IF NOT EXISTS `t_channelDetailed_201712` (
  `tid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `uid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '����ID',
  `pid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '������ID',
  `imei` varchar(256) NOT NULL DEFAULT '' COMMENT '�ֻ�ΨһӲ����',
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '���ID',
  `phone` varchar(50) NOT NULL DEFAULT '' COMMENT '�ֻ����� ios or android',
  `ip` varchar(50) NOT NULL DEFAULT '' COMMENT '�û�IP',
  `times` int(10) unsigned NOT NULL DEFAULT '0',
  `times_index` int(10) unsigned NOT NULL DEFAULT '0',
  `Rate` tinyint(4) NOT NULL DEFAULT '0' COMMENT '��ǰ�����ʱ���',
  `effect` tinyint(1) NOT NULL DEFAULT '0' COMMENT '1 ���� 0 ����',
  PRIMARY KEY (`tid`),
  KEY `guid` (`guid`),KEY `times` (`times`),
  KEY `uid` (`uid`),KEY `pid` (`pid`),
  KEY `times_index` (`times_index`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 COMMENT '������װ��ϸ��';

DROP TABLE IF EXISTS `t_channelDetailed_201801`;
CREATE TABLE IF NOT EXISTS `t_channelDetailed_201801`(
  `tid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `uid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '����ID',
  `pid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '������ID',
  `imei` varchar(256) NOT NULL DEFAULT '' COMMENT '�ֻ�ΨһӲ����',
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '���ID',
  `phone` varchar(50) NOT NULL DEFAULT '' COMMENT '�ֻ����� ios or android',
  `ip` varchar(50) NOT NULL DEFAULT '' COMMENT '�û�IP',
  `times` int(10) unsigned NOT NULL DEFAULT '0',
  `times_index` int(10) unsigned NOT NULL DEFAULT '0',
  `Rate` tinyint(4) NOT NULL DEFAULT '0' COMMENT '��ǰ�����ʱ���',
  `effect` tinyint(1) NOT NULL DEFAULT '0' COMMENT '1 ���� 0 ����',
  PRIMARY KEY (`tid`),
  KEY `guid` (`guid`),KEY `times` (`times`),
  KEY `uid` (`uid`),KEY `pid` (`pid`),
  KEY `times_index` (`times_index`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 COMMENT '������װ��ϸ��';

DROP TABLE IF EXISTS `t_channelDetailed_201802`;
CREATE TABLE IF NOT EXISTS `t_channelDetailed_201802` (
  `tid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `uid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '����ID',
  `pid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '������ID',
  `imei` varchar(256) NOT NULL DEFAULT '' COMMENT '�ֻ�ΨһӲ����',
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '���ID',
  `phone` varchar(50) NOT NULL DEFAULT '' COMMENT '�ֻ����� ios or android',
  `ip` varchar(50) NOT NULL DEFAULT '' COMMENT '�û�IP',
  `times` int(10) unsigned NOT NULL DEFAULT '0',
  `times_index` int(10) unsigned NOT NULL DEFAULT '0',
  `Rate` tinyint(4) NOT NULL DEFAULT '0' COMMENT '��ǰ�����ʱ���',
  `effect` tinyint(1) NOT NULL DEFAULT '0' COMMENT '1 ���� 0 ����',
  PRIMARY KEY (`tid`),
  KEY `guid` (`guid`),KEY `times` (`times`),
  KEY `uid` (`uid`),KEY `pid` (`pid`),
  KEY `times_index` (`times_index`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 COMMENT '������װ��ϸ��';


DROP TABLE IF EXISTS `t_channel_form`;
CREATE TABLE `t_channel_form` (
  `tid` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `uid` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '����ID',
  `ck_pid` tinyint(1) NOT NULL DEFAULT '0' COMMENT '�����ȼ�',
  `father_id` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '����ID',
  `times` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '����',
  `register_num` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'ע����',
  `shi_num` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '����',
  `Bank_num` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '�ֻ�����',
  `Pay_num` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '��ֵ����',
  `Pay_money` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '��ֵ���',
  `tax` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '��Ϸ˰��',
  `CommissionRate` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '�����',
  `price` double(9,2) NOT NULL DEFAULT '0.00' COMMENT '��װ�������õ��ۼƷ�',
  `chargeType` tinyint(1) NOT NULL DEFAULT '0' COMMENT '�Ʒ����� 1 ��װ�Ʒ� 2 ���',
  `Pay_ck` tinyint(2) NOT NULL DEFAULT '0' COMMENT '�Ƿ�֧��',
  `Cheat` tinyint(2) NOT NULL DEFAULT '0' COMMENT '�Ƿ�����',
   PRIMARY KEY (`tid`),
   KEY `uid` (`uid`),KEY `father_id` (`father_id`),
   KEY `ck_pid` (`ck_pid`),
   KEY `times` (`times`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 COMMENT '��������';

DROP TABLE IF EXISTS `t_channel_total`;
CREATE TABLE `t_channel_total` (
  `tid` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `uid` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '����ID',
  `register_num` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'ע����',
  `Bank_num` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '֧���ʺŰ���',
  `Pay_num` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '��ֵ����',
  `Pay_money` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '��ֵ���',
  `tax` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '��Ϸ˰��',
  `CommissionRate` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '�����',
  `income` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '����',
  cache_times int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '����ʱ��',
   PRIMARY KEY (`tid`),
   KEY `uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 COMMENT '�����ܱ���';