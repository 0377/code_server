USE `frame`;
CREATE TABLE `complaints` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `account` varchar(100) DEFAULT '' COMMENT '����˺�',
  `agent_id` int(10) DEFAULT NULL COMMENT '��Ͷ�ߴ���/�ͷ�ID',
  `type` tinyint(2) NOT NULL DEFAULT '0' COMMENT '0Ͷ�߿ͷ� 1Ͷ�ߴ���',
  `product` varchar(255) NOT NULL DEFAULT '' COMMENT 'Ͷ��������Ʒ ��� GG ���� �齫 ����',
  `content` varchar(1000) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'Ͷ������',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=MyISAM AUTO_INCREMENT=12 DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Ͷ�߱�'
