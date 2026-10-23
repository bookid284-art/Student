#include "NightSchool/NightSchoolStory.h"

namespace NightSchoolStory
{
	const TCHAR* ChapterBanner(int32 Chapter)
	{
		switch (Chapter)
		{
		case 1: return TEXT("第一章  未接。电话先响。镜头还亮着。");
		case 2: return TEXT("第二章  空位。没有人把书包放上来。");
		case 3: return TEXT("第三章  请客。硬币只负责让你看见那本账。");
		case 4: return TEXT("第四章  恨铁。思路在，后两问空着。");
		case 5: return TEXT("第五章  榜。名字被念过，也被人写在栏外。");
		case 6: return TEXT("第六章  两扇门。看清自己，再决定留下或离开。");
		default: return TEXT("里屋");
		}
	}

	const TCHAR* CallTitleA() { return TEXT("通话记录 · 清晨"); }
	const TCHAR* CallBodyA()
	{
		return TEXT("许桂芳的声音很清醒。她问昨夜刷了几页，问定位为什么在宿舍而不是教室。她没有问睡了没有。\n\n她说手机不要关定位。关了她会再打来。你没有接。录音把后半句留在这里：夏夏，妈妈只有你一个。\n\n这不是第一通。西边这间通话间把没被接起来的声音收成了纸。");
	}

	const TCHAR* CallTitleB() { return TEXT("通话记录 · 锁屏"); }
	const TCHAR* CallBodyB()
	{
		return TEXT("第二通更短。她说屏幕她可以从自己那边锁上，锁上你就没法再看那些没用的东西。她说的没用，包括同学传来的一张课表。\n\n你后来在洗衣间看见林小满的课表。那是另一章的东西。这一页只记下：电子设备不在你手里，在她手里。\n\n她没有骂出声。安静比骂更占地方。");
	}

	const TCHAR* CallTitleC() { return TEXT("通话记录 · 镜头"); }
	const TCHAR* CallBodyC()
	{
		return TEXT("第三通她已经不跟你说了。她像在跟一个会记下来的人说：摄像头对着床就行，我只要看见他在。\n\n老吴的记录里写过，他当天没有立刻答应。后来还是装了。这一页把那句“只要看见他在”留在抽屉外面，是为了让你自己听见。\n\n三页都读完，再去看值班室。布还在抽屉里。");
	}

	const TCHAR* ClassLogTitleA() { return TEXT("班级日志 · 座位"); }
	const TCHAR* ClassLogBodyA()
	{
		return TEXT("值日生画了座位。许知夏的名字旁边没有第二个名字。线被尺压得很直，像空着是一种安排。\n\n页脚有人用铅笔加了半句，又擦掉了。擦不干净的地方还留着“可以坐”。没有署名。江晚的条子后来解释了这半句，可日志本身只负责把空位画出来。");
	}

	const TCHAR* ClassLogTitleB() { return TEXT("班级日志 · 奶茶"); }
	const TCHAR* ClassLogBodyB()
	{
		return TEXT("集资单上有一栏写着许知夏，金额是别人的笔迹。日期是他去校医室量血压的那天下午。人没在小卖部，名字在。\n\n没有人在这一栏旁边写“他答应了”。没有人需要他答应。这就是请客在教室里的样子：先写上，再等他看见。");
	}

	const TCHAR* ClassLogTitleC() { return TEXT("班级日志 · 实验"); }
	const TCHAR* ClassLogBodyC()
	{
		return TEXT("分组表贴着化学实验。许知夏的组员一栏空着。梁老师在空栏外写了个问号，没有再填人。\n\n问号不是关心。问号是他知道没有人愿意同组，又把这件事交给纸。你把三页都看完的时候，空位还在教室东边。条子齐了才能坐下。");
	}

	const TCHAR* ReceiptTitleA() { return TEXT("收据 · 已付"); }
	const TCHAR* ReceiptBodyA()
	{
		return TEXT("小票皱着。品名是一杯奶茶，付款人写成许知夏。金额和铁盒里那枚硬币对不上。硬币只够看这张纸，不够付这杯。\n\n已付两个字是盖章盖上去的。章不是你的。后仓把这种纸收在卷帘后面，免得柜台前的人一边笑一边对账。");
	}

	const TCHAR* ReceiptTitleB() { return TEXT("收据 · 缺席"); }
	const TCHAR* ReceiptBodyB()
	{
		return TEXT("第二张的时间，你在宿舍接一通未接来电，人没有下楼。小票仍然写着你的名字，后面跟着“下次双倍”。\n\n下次两个字比金额重。它让请客变成一条会自己长出来的走廊。你把纸叠好。它该和账本一起被看见，而不是再被收进后仓。");
	}

	const TCHAR* ReceiptTitleC() { return TEXT("收据 · 背面"); }
	const TCHAR* ReceiptBodyC()
	{
		return TEXT("背面有一行很小的字：我没有划掉。划掉以后下一行会写成我。——晚\n\n江晚没有把这张收据交给你。她把它塞进后仓，像把不敢说的话放在一个需要硬币才进得去的地方。三张都看完，柜台下的账本才肯被拿走。");
	}

	const TCHAR* DraftTitleA() { return TEXT("改卷批注 · 一"); }
	const TCHAR* DraftBodyA()
	{
		return TEXT("梁老师的红笔停在一道大题中间。前面的式子是对的。后面空着。他写：会，但不写完。\n\n这不是评语该有的句子。这是他跟自己生气。纸没有把气变成帮助。你只是第一次看见恨字写在公式旁边。");
	}

	const TCHAR* DraftTitleB() { return TEXT("改卷批注 · 二"); }
	const TCHAR* DraftBodyB()
	{
		return TEXT("第二页是课堂记录，不是分数。他写：点到许知夏，他看桌面。再点一次，他还是看桌面。我让全班等他。\n\n全班等他的那几秒，就是后来没有人愿意同桌的其中一秒。梁老师把等待记成不争气。他没有记下自己为什么要让全班等。");
	}

	const TCHAR* DraftTitleC() { return TEXT("改卷批注 · 三"); }
	const TCHAR* DraftBodyC()
	{
		return TEXT("第三页是给自己的。梁老师写：家长会上我说摄像头有助于学习。我说完就后悔了半句，没有收回。\n\n半句后悔还是恨。它解释不了镜头，只说明他也知道那句话不该出口。三页看完，他留在桌上的字条才会把年级组打开。");
	}

	const TCHAR* NoticeTitleE() { return TEXT("长榜 · 五"); }
	const TCHAR* NoticeBodyE()
	{
		return TEXT("北廊的四张还不够。长榜把同一栏又印了一遍，字更小，名字更多。许知夏夹在两个更高的分数中间，像被排进来是为了让旁边的人好念。\n\n你不用再算自己排第几。榜不因你看完就重排。它只是不再只贴在你看不见的那一面。");
	}

	const TCHAR* NoticeTitleF() { return TEXT("长榜 · 六"); }
	const TCHAR* NoticeBodyF()
	{
		return TEXT("有人用手机拍过空位。照片没有贴在这里，说明文字还在：座位长期空置，建议班主任关注。关注两个字下面没有写你的名字，可座位号是你的。\n\n关注如果只变成另一只镜头，就还是第一章里那通没接的电话。");
	}

	const TCHAR* NoticeTitleG() { return TEXT("长榜 · 七"); }
	const TCHAR* NoticeBodyG()
	{
		return TEXT("学号还在，姓名被刮去了一层。刮的人没有留下自己的名字。刮完以后，号码仍旧指向你。\n\n周先生后来不会叫你学号。这一张只让你看见，有人曾经以为抹掉两个字，人就会从栏里消失。人没有消失。栏还在。");
	}

	const TCHAR* NoticeTitleH() { return TEXT("长榜 · 八"); }
	const TCHAR* NoticeBodyH()
	{
		return TEXT("最后一张很短：校医室不按名次开门。\n\n八张都读完的时候，章印会回到你身上。它要送回宿舍，送到那只已经盖住的镜头旁边。不是再被看一次。是把这一章从榜上拿下来，亲手放到你自己的床位边上。");
	}

	const TCHAR* ChartTitleA() { return TEXT("旧病历 · 物品"); }
	const TCHAR* ChartBodyA()
	{
		return TEXT("周先生的字很慢。他写：林小满毕业时没有回来取铅笔盒、月票和课表。人已经离开这条线。东西还在楼里。\n\n这不是你的病历。这是一间屋还扣着别人的日常。你来取，不是为了变成她，是为了让扣着的东西有个托盘可以放下。");
	}

	const TCHAR* ChartTitleB() { return TEXT("旧病历 · 血压"); }
	const TCHAR* ChartBodyB()
	{
		return TEXT("另一页全是周先生自己的血压。数字普通。他在空白处写：这些我量得了。电话我量不了。笑声我量不了。\n\n他没有给你开任何需要藏起来的东西。这一页只说明，校医室能做的事很有限，所以他才只说两扇门，不说药。");
	}

	const TCHAR* ChartTitleC() { return TEXT("旧病历 · 门"); }
	const TCHAR* ChartBodyC()
	{
		return TEXT("第三页只有四行。两扇门不是治疗。一扇要你看见自己还站在光里。一扇从里面带上，人就留在听不见的地方。\n\n他写：先把三样旧物放到托盘上，我再把话说完。说完以前，不要去碰开关，也不要去碰柜子里的锁。\n\n三页都看完，才能叫他。");
	}

	const TCHAR* StampIdle()
	{
		return TEXT("这里还没有你的章印。每一章结束时，章印会回到你身上，要盖在这一章指定的地方。盖完，下一章的门才认。");
	}

	const TCHAR* StampDone1()
	{
		return TEXT("第一章的章印按进了前厅的木板。走廊的门松了。门外会有人走。你没有可以打回去的东西。\n\n下一章在东边的教室。空位、条子、班级日志，都在那边。");
	}

	const TCHAR* StampDone2()
	{
		return TEXT("第二章的章印盖在值班室。老吴的桌子没有拒绝它。空位那件事算你自己坐过了。\n\n现在可以把账钉出去。钉之前，后仓的三张收据要先看完。");
	}

	const TCHAR* StampDone3()
	{
		return TEXT("第三章的章印盖进了通话间。那些没接的电话还在纸上，章印没有把它们擦掉。\n\n办公室现在肯收卷子。分数仍然不会改。");
	}

	const TCHAR* StampDone4()
	{
		return TEXT("第四章的章印盖回了那张空位。恨字还在批注里。章印不替梁老师收回那句话。\n\n北面长榜的门可以开了。八张告示都要看。");
	}

	const TCHAR* StampDone5()
	{
		return TEXT("第五章的章印放回了盖住的镜头旁边。红点还在布底下跳。榜上的名字没有因此消失。\n\n校医室现在可以进。先看周先生的三页旧病历，再叫他。");
	}

	const TCHAR* SelfTitle() { return TEXT("你自己"); }
	const TCHAR* SelfBody()
	{
		return TEXT("光里站着一个穿校服的人。短发，黑框眼镜，书包还在肩上，手里是一本薄册子。他偏瘦，十七八岁，脸上没有伤。\n\n他就是你。他不说话，也不替你拨开关。开关在旁边，要用你自己的手。西边的柜子仍然可以从里面带上。\n\n看见他，不算离开，也不算留下。看见只是这一章剩下的那一步。");
	}

	const TCHAR* NeedSelf()
	{
		return TEXT("开关是温的。光里那个人还没被你看清。先走到他面前。他不会替你选。");
	}

	const TCHAR* StoreLocked()
	{
		return TEXT("后仓的门从卷帘这边才开得了。硬币还没送进小卖部的时候，这里是暗的。");
	}

	const TCHAR* AnnexLocked()
	{
		return TEXT("长榜还关着。年级组的三页要读完，第四章的章印要先盖回教室那张空位。榜不因为你站在门口就自己打开。");
	}

	const TCHAR* NeedReceipts()
	{
		return TEXT("账本还钉在柜台下。后仓里有三张收据，看完它才肯被拿走。收据不是再付一次钱。");
	}

	const TCHAR* NeedDrafts()
	{
		return TEXT("字条可以读。年级组先不开。办公室里还有三页改卷批注，看完再来。");
	}
}
