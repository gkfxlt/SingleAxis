﻿#include<iostream>
#include<aris.hpp>
#include"moveEA.h"

using namespace std;
//调用aris库中的plan模块
using namespace aris::plan;

//创建ethercat主站控制器controller，并根据xml文件添加从站信息
auto createControllerRokaeXB4()->std::unique_ptr<aris::control::Controller>	
{
	std::unique_ptr<aris::control::Controller> controller(new aris::control::EthercatController);
	
	std::string xml_str =
        "<EthercatMotion phy_id=\"0\" product_code=\"0x00000000\""
        " vendor_id=\"0x000002e1\" revision_num=\"0x00029001\" dc_assign_activate=\"0x0300\""
        " min_pos=\"-0.40\" max_pos=\"0.45\" max_vel=\"0.3\" min_vel=\"-0.3\""
		" max_acc=\"2.0\" min_acc=\"-2.0\" max_pos_following_error=\"0.005\" max_vel_following_error=\"0.005\""
        " home_pos=\"0\" pos_factor=\"-10000000\" pos_offset=\"0.0\">"
		"	<SyncManagerPoolObject>"
		"		<SyncManager is_tx=\"false\"/>"
		"		<SyncManager is_tx=\"true\"/>"
		"		<SyncManager is_tx=\"false\">"
		"			<Pdo index=\"0x1600\" is_tx=\"false\">"
		"				<PdoEntry name=\"control_word\" index=\"0x6040\" subindex=\"0x00\" size=\"16\"/>"
		"				<PdoEntry name=\"mode_of_operation\" index=\"0x6060\" subindex=\"0x00\" size=\"8\"/>"
		"				<PdoEntry name=\"target_pos\" index=\"0x607A\" subindex=\"0x00\" size=\"32\"/>"
		"				<PdoEntry name=\"target_vel\" index=\"0x60FF\" subindex=\"0x00\" size=\"32\"/>"
		"				<PdoEntry name=\"offset_vel\" index=\"0x60B1\" subindex=\"0x00\" size=\"32\"/>"
		"				<PdoEntry name=\"targer_tor\" index=\"0x6071\" subindex=\"0x00\" size=\"16\"/>"
		"				<PdoEntry name=\"offset_tor\" index=\"0x60B2\" subindex=\"0x00\" size=\"16\"/>"
		"			</Pdo>"
		"		</SyncManager>"
		"		<SyncManager is_tx=\"true\">"
		"			<Pdo index=\"0x1A00\" is_tx=\"true\">"
		"				<PdoEntry name=\"status_word\" index=\"0x6041\" subindex=\"0x00\" size=\"16\"/>"
		"				<PdoEntry name=\"mode_of_display\" index=\"0x6061\" subindex=\"0x00\" size=\"8\"/>"
		"				<PdoEntry name=\"pos_actual_value\" index=\"0x6064\" subindex=\"0x00\" size=\"32\"/>"
		"				<PdoEntry name=\"vel_actual_value\" index=\"0x606c\" subindex=\"0x00\" size=\"32\"/>"
		"				<PdoEntry name=\"cur_actual_value\" index=\"0x6078\" subindex=\"0x00\" size=\"16\"/>"
		"			</Pdo>"
		"		</SyncManager>"
		"	</SyncManagerPoolObject>"
		"</EthercatMotion>";
	controller->slavePool().add<aris::control::EthercatMotion>().loadXmlStr(xml_str);
	

    //controller->slavePool().add<aris::control::EthercatSlave>();
    //controller->slavePool().back().setPhyId(0);
	dynamic_cast<aris::control::EthercatSlave&>(controller->slavePool().back()).scanInfoForCurrentSlave();
	dynamic_cast<aris::control::EthercatSlave&>(controller->slavePool().back()).scanPdoForCurrentSlave();
	dynamic_cast<aris::control::EthercatSlave&>(controller->slavePool().back()).setDcAssignActivate(0x300);



    //添加EtherCAT Node,添加顺序与物理拓扑顺序一致,
     controller->slavePool().add<aris::control::EthercatSlave>();
     controller->slavePool().back().setPhyId(1);
     dynamic_cast<aris::control::EthercatSlave&>(controller->slavePool().back()).scanInfoForCurrentSlave();
     dynamic_cast<aris::control::EthercatSlave&>(controller->slavePool().back()).scanPdoForCurrentSlave();
     dynamic_cast<aris::control::EthercatSlave&>(controller->slavePool().back()).setDcAssignActivate(0x300);

        //添加第一个EtherCAT Node
     controller->slavePool().add<aris::control::EthercatSlave>();
     controller->slavePool().back().setPhyId(2);
     dynamic_cast<aris::control::EthercatSlave&>(controller->slavePool().back()).scanInfoForCurrentSlave();
     dynamic_cast<aris::control::EthercatSlave&>(controller->slavePool().back()).scanPdoForCurrentSlave();
        //添加第二个EtherCAT从站
     controller->slavePool().add<aris::control::EthercatSlave>();
     controller->slavePool().back().setPhyId(3);
     dynamic_cast<aris::control::EthercatSlave&>(controller->slavePool().back()).scanInfoForCurrentSlave();
     dynamic_cast<aris::control::EthercatSlave&>(controller->slavePool().back()).scanPdoForCurrentSlave();


    /*
	dynamic_cast<aris::control::Motion&>(controller->slavePool()[0]).setHomePos(0);
	dynamic_cast<aris::control::Motion&>(controller->slavePool()[0]).setPosOffset(0);
	dynamic_cast<aris::control::Motion&>(controller->slavePool()[0]).setPosFactor(804140);
	dynamic_cast<aris::control::Motion&>(controller->slavePool()[0]).setMaxPos(6.28);
	dynamic_cast<aris::control::Motion&>(controller->slavePool()[0]).setMinPos(-6.28);
	dynamic_cast<aris::control::Motion&>(controller->slavePool()[0]).setMaxVel(12.56);
	dynamic_cast<aris::control::Motion&>(controller->slavePool()[0]).setMinVel(-12.56);
	dynamic_cast<aris::control::Motion&>(controller->slavePool()[0]).setMaxAcc(60);
	dynamic_cast<aris::control::Motion&>(controller->slavePool()[0]).setMinAcc(-60);
	dynamic_cast<aris::control::Motion&>(controller->slavePool()[0]).setMaxPosFollowingError(0.1);
	dynamic_cast<aris::control::Motion&>(controller->slavePool()[0]).setMaxVelFollowingError(0.5);
	*/
	return controller;
};


// 单关节正弦往复轨迹 //
struct MoveJSParam
{
	double j1;
	double time;
	uint32_t timenum;
};
auto MoveJS::prepairNrt(const std::map<std::string, std::string> &params, PlanTarget &target)->void
{
	MoveJSParam param;

	param.j1 = 0.0;
	param.time = 0.0;
	param.timenum = 0;

	for (auto &p : params)
	{
		if (p.first == "j1")
		{
			if (p.second == "current_pos")
			{
				param.j1 = target.controller->motionPool()[0].actualPos();
			}
			else
			{
				param.j1 = std::stod(p.second);
			}

		}
		else if (p.first == "time")
		{
			param.time = std::stod(p.second);
		}
		else if (p.first == "timenum")
		{
			param.timenum = std::stoi(p.second);
		}
	}
	target.param = param;

	std::fill(target.mot_options.begin(), target.mot_options.end(),
		Plan::USE_TARGET_POS);

}
auto MoveJS::executeRT(PlanTarget &target)->int
{
	auto &param = std::any_cast<MoveJSParam&>(target.param);
	auto time = static_cast<int32_t>(param.time * 1000);
	auto totaltime = static_cast<int32_t>(param.timenum * time);
	static double begin_pjs;
	static double step_pjs;

	if ((1 <= target.count) && (target.count <= time / 2))
	{
		// 获取当前起始点位置 //
		if (target.count == 1)
		{
			begin_pjs = target.controller->motionPool()[0].actualPos();
			step_pjs = target.controller->motionPool()[0].actualPos();
		}
		step_pjs = begin_pjs + param.j1 * (1 - std::cos(2 * PI*target.count / time)) / 2;
		target.controller->motionPool().at(0).setTargetPos(step_pjs);
	}
	else if ((time / 2 < target.count) && (target.count <= totaltime - time / 2))
	{
		// 获取当前起始点位置 //
		if (target.count == time / 2 + 1)
		{
			begin_pjs = target.controller->motionPool()[0].actualPos();
			step_pjs = target.controller->motionPool()[0].actualPos();
		}

		step_pjs = begin_pjs - 2 * param.j1 * (1 - std::cos(2 * PI*(target.count - time / 2) / time)) / 2;
		target.controller->motionPool().at(0).setTargetPos(step_pjs);
	}
	else if ((totaltime - time / 2 < target.count) && (target.count <= totaltime))
	{
		// 获取当前起始点位置 //
		if (target.count == totaltime - time / 2 + 1)
		{
			begin_pjs = target.controller->motionPool()[0].actualPos();
			step_pjs = target.controller->motionPool()[0].actualPos();
		}
		step_pjs = begin_pjs - param.j1 * (1 - std::cos(2 * PI*(target.count - totaltime + time / 2) / time)) / 2;
		target.controller->motionPool().at(0).setTargetPos(step_pjs);
	}

	if (target.model->solverPool().at(1).kinPos())return -1;

	// 访问主站 //
	auto controller = target.controller;

	// 打印电流 //
	auto &cout = controller->mout();
	if (target.count % 100 == 0)
	{
		cout << "pos"  << ":" << controller->motionAtAbs(0).actualPos() << "  ";
		cout << "vel"  << ":" << controller->motionAtAbs(0).actualVel() << "  ";
		cout << "cur"  << ":" << controller->motionAtAbs(0).actualCur() << "  ";

		cout << std::endl;
	}

	// log 电流 //
	auto &lout = controller->lout();
	lout << controller->motionAtAbs(0).targetPos() << ",";
	lout << controller->motionAtAbs(0).actualPos() << ",";
	lout << controller->motionAtAbs(0).actualVel() << ",";
	lout << controller->motionAtAbs(0).actualCur() << ",";
	lout << std::endl;

	return totaltime - target.count;
}
auto MoveJS::collectNrt(PlanTarget &target)->void {}
MoveJS::MoveJS(const std::string &name) :Plan(name)
{
	command().loadXmlStr(
		"<Command name=\"moveJS\">"
		"	<GroupParam>"
		"		<Param name=\"j1\" default=\"current_pos\"/>"
		"		<Param name=\"time\" default=\"1.0\" abbreviation=\"t\"/>"
		"		<Param name=\"timenum\" default=\"2\" abbreviation=\"n\"/>"
		"	</GroupParam>"
		"</Command>");
}


// moveEAP //
struct MoveEAPParam
{
	double axis_begin_pos;
	double axis_pos;
	double axis_vel;
	double axis_acc;
	double axis_dec;
	bool abs;
};
auto MoveEAP::prepairNrt(const std::map<std::string, std::string> &params, PlanTarget &target)->void
{
    auto c = target.controller;
	MoveEAPParam param;
    param.axis_begin_pos = 0.0;

	for (auto &p : params)
	{
		if (p.first == "pos")
		{
			param.axis_pos = std::stod(p.second);
		}
		else if (p.first == "vel")
		{	
			param.axis_vel = std::stod(p.second);
		}
		else if (p.first == "acc")
		{
			param.axis_acc = std::stod(p.second);
		}
		else if (p.first == "dec")
		{
			param.axis_dec = std::stod(p.second);
		}
		else if (p.first == "ab")
		{
			param.abs = std::stoi(p.second);
		}
	}

	target.param = param;
	std::string ret = "ok";
	target.ret = ret;

    for(auto &option:target.mot_options) option|=
           Plan::USE_TARGET_POS |
           //#ifdef WIN32
           Plan::NOT_CHECK_POS_MIN |
           Plan::NOT_CHECK_POS_MAX |
           Plan::NOT_CHECK_POS_CONTINUOUS |
           Plan::NOT_CHECK_POS_CONTINUOUS_AT_START |
           Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER |
           Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER_AT_START |
           Plan::NOT_CHECK_POS_FOLLOWING_ERROR |
           //#endif
           Plan::NOT_CHECK_VEL_MIN |
           Plan::NOT_CHECK_VEL_MAX |
           Plan::NOT_CHECK_VEL_CONTINUOUS |
           Plan::NOT_CHECK_VEL_CONTINUOUS_AT_START |
           Plan::NOT_CHECK_VEL_FOLLOWING_ERROR|
           Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER |
           Plan::NOT_CHECK_POS_CONTINUOUS_SECOND_ORDER_AT_START |
           Plan::NOT_CHECK_ENABLE;

}
auto MoveEAP::executeRT(PlanTarget &target)->int
{
	auto &param = std::any_cast<MoveEAPParam&>(target.param);
	// 访问主站 //
    auto controller = target.controller;

    auto Sencontroller = dynamic_cast<aris::control::EthercatController*>(target.controller);

    int16_t rawData;
    double volToSig = 10.0;
    static double sF0=0,sF1=0,Force=0,Force0=0,begin_pjs=0,sum_bjs=0,step_bjs=0;

    Sencontroller->slavePool().at(2).readPdo(0x6000, 0x11, &rawData, 16);
    Force= volToSig *rawData/65535.0*522.5;

    if (target.count == 1)
    {

        Force0=Force;
        begin_pjs=controller->motionAtAbs(0).actualPos();
    }
    Force=Force-Force0;

    double CutFreq=10,intDT=0.001;
    sF1=sF0+intDT*(Force-sF0)*CutFreq;

    sum_bjs=sum_bjs+(sF1-5)/300/1000;

    step_bjs=begin_pjs+sum_bjs;

    if(abs(step_bjs)<0.4)
          controller->motionAtAbs(0).setTargetPos(step_bjs);

	// 每1000ms打印 目标位置、实际位置、实际速度、实际电流 //
	auto &cout = controller->mout();
	if (target.count % 1000 == 0)
	{
        cout << step_bjs << "  "
            << controller->motionAtAbs(0).actualPos()<<"  "
           << Force<<"  "
             <<sF1<<std::endl;
	}
	// log 目标位置、实际位置、实际速度、实际电流 //
		
	auto &lout = controller->lout();
    lout << controller->motionAtAbs(0).targetPos() << "  "
        << controller->motionAtAbs(0).actualPos() << "  ";
    lout << std::endl;
	// 返回total_count - target.count给aris实时核，值为-1，报错；值为0，结束；值大于0，继续执行下一个count

    sF0=sF1;
    return 1000000 - target.count;
}
auto MoveEAP::collectNrt(PlanTarget &target)->void {}
MoveEAP::MoveEAP(const std::string &name) :Plan(name)
{
	command().loadXmlStr(
		"<Command name=\"moveEAP\">"
		"	<GroupParam>"
		"		<Param name=\"begin_pos\" default=\"0.1\" abbreviation=\"b\"/>"
		"		<Param name=\"pos\" default=\"0.1\"/>"
		"		<Param name=\"vel\" default=\"0.02\"/>"
		"		<Param name=\"acc\" default=\"0.3\"/>"
		"		<Param name=\"dec\" default=\"-0.3\"/>"
		"		<Param name=\"ab\" default=\"0\"/>"
		"	</GroupParam>"
		"</Command>");
}


// 电缸电流控制 //
struct MoveEACParam
{
	double begin_pos, pos, static_vel, kp_p, kp_v, ki_v;
	bool ab;
};
static std::atomic_bool enable_moveEAC = true;
auto MoveEAC::prepairNrt(const std::map<std::string, std::string> &params, PlanTarget &target)->void
{
	MoveEACParam param;
	for (auto &p : params)
	{
		if (p.first == "pos")
		{
			param.pos = std::stod(p.second);
		}
		else if (p.first == "static_vel")
		{
			param.static_vel = std::stod(p.second);
		}
		else if (p.first == "ab")
		{
			param.ab = std::stod(p.second);
		}
		else if (p.first == "kp_p")
		{
			param.kp_p = std::stod(p.second);
		}
		else if (p.first == "kp_v")
		{
			param.kp_v = std::stod(p.second);
		}
		else if (p.first == "ki_v")
		{
			param.ki_v = std::stod(p.second);
		}
	}
	target.param = param;
	//std::fill(target.mot_options.begin(), target.mot_options.end(), Plan::USE_TARGET_POS);

}
auto MoveEAC::executeRT(PlanTarget &target)->int
{
	auto &param = std::any_cast<MoveEACParam&>(target.param);
	// 访问主站 //
	auto controller = target.controller;
	bool is_running{ true };
	bool ds_is_all_finished{ true };
	bool md_is_all_finished{ true };

	//第一个周期，将目标电机的控制模式切换到电流控制模式
	if (target.count == 1)
	{
		is_running = true;
		controller->motionPool().at(0).setModeOfOperation(10);	//切换到电流控制
	}

	//最后一个周期将目标电机去使能
	if (!enable_moveEAC)
	{
		is_running = false;
	}
	if (!is_running)
	{
		auto ret = controller->motionPool().at(0).disable();
		if (ret)
		{
			ds_is_all_finished = false;
		}
	}

	//将目标电机由电流模式切换到位置模式
	if (!is_running&&ds_is_all_finished)
	{
		auto &cm = controller->motionPool().at(0);
		controller->motionPool().at(0).setModeOfOperation(8);
		auto ret = cm.mode(8);
		cm.setTargetPos(cm.actualPos());
		if (ret)
		{
			md_is_all_finished = false;
		}
	}

	//标记采用那一段公式计算压力值//
	int phase;
	double fore_cur = 0, force = 0, ft_pid;

	//力控算法//
	if (is_running)
	{
		//参数定义//		
		double pt, pa, vt, va, voff, ft, foff;
		static double v_integral = 0.0;
		pa = controller->motionAtAbs(0).actualPos();
		va = controller->motionAtAbs(0).actualVel();
		pt = param.pos;
		vt = 0.0;
		voff = vt * 1000;
		foff = 0.0;

		//位置环//
		{
			vt = param.kp_p*(pt - pa) + voff;
			//限制速度的范围在-0.01~0.01之间
			vt = std::max(-0.01, vt);
			vt = std::min(0.01, vt);
		}

		//速度环//
		{
			v_integral = v_integral + vt - va;
			ft = param.kp_v*(vt - va) + param.ki_v * v_integral + foff;
			//限制电流的范围在-100~100(千分数：额定电流是1000)之间
			ft = std::max(-100.0, ft);
			ft = std::min(100.0, ft);
			ft_pid = ft;
		}

		//根据电流值换算压力值//
		{
			double ff = 0, fc, fg, fs;
			fc = controller->motionAtAbs(0).actualCur() * ea_index;
			fg = ea_gra;
			fs = std::abs(ea_c * ea_index);
			if (std::abs(controller->motionAtAbs(0).actualVel()) > param.static_vel)
			{
				if (controller->motionAtAbs(0).actualVel() > 0)
				{
					ff = (-ea_a * controller->motionAtAbs(0).actualVel()*controller->motionAtAbs(0).actualVel() + ea_b * controller->motionAtAbs(0).actualVel() + ea_c) * ea_index;
					force = ff + fg + fc;
					phase = 1;
					fore_cur = (-ff - fg) / ea_index;
					//fore_cur = (1810 * a * 1000 * 1000 - ff - fg) / ea_index;
				}
				else
				{
					ff = (ea_a * controller->motionAtAbs(0).actualVel()*controller->motionAtAbs(0).actualVel() + ea_b * controller->motionAtAbs(0).actualVel() - ea_c) * ea_index;
					force = ff + fg + fc;
					phase = 2;
					fore_cur = (-ff - fg) / ea_index;
				}
			}
			else
			{
				if (std::abs(fc + fg) <= fs)
				{
					force = 0;
					phase = 3;
					fore_cur = 0.0;
				}
				else
				{
					if (fc + fg < -fs)
					{
						force = fc + fg + fs;
						phase = 4;
						fore_cur = (-fg - fs) / ea_index;
					}
					else
					{
						force = fc + fg - fs;;
						phase = 5;
						fore_cur = (-fg + fs) / ea_index;;
					}
				}
			}
			fore_cur = std::max(-100.0, fore_cur);
			fore_cur = std::min(100.0, fore_cur);

		}

		double weight = 1;
		controller->motionAtAbs(0).setTargetCur(ft_pid + weight * fore_cur);
	}

	// print //
	auto &cout = controller->mout();
	if (target.count % 100 == 0)
	{
		cout << phase << "  "
			<< force << "  "
			<< fore_cur << "  "
			<< ft_pid << "  "
			<< controller->motionAtAbs(0).actualPos() << "  "
			<< controller->motionAtAbs(0).actualVel() << "  "
			<< controller->motionAtAbs(0).actualCur() << "  "
			<< std::endl;
	}

	// log //
	auto &lout = controller->lout();
	{
		lout << phase << "  "
			<< force << "  "
			<< fore_cur << "  "
			<< ft_pid << "  "
			<< controller->motionAtAbs(0).actualPos() << "  "
			<< controller->motionAtAbs(0).actualVel() << "  "
			<< controller->motionAtAbs(0).actualCur() << "  "
			<< std::endl;
	}

	return (!is_running&&ds_is_all_finished&&md_is_all_finished) ? 0 : 1;
}
auto MoveEAC::collectNrt(PlanTarget &target)->void {}
MoveEAC::MoveEAC(const std::string &name) :Plan(name)
{
	command().loadXmlStr(
		"<Command name=\"moveEAC\">"
		"	<GroupParam>"
		"		<Param name=\"pos\" default=\"0.01\"/>"
		"		<Param name=\"static_vel\" default=\"0.001\"/>"
		"		<Param name=\"ab\" default=\"0\"/>"
		"		<Param name=\"kp_p\" default=\"0.1\"/>"
		"		<Param name=\"kp_v\" default=\"5\"/>"
		"		<Param name=\"ki_v\" default=\"0.1\"/>"
		"	</GroupParam>"
		"</Command>");
}


// moveStop，切出电流控制 //
auto MoveStop::prepairNrt(const std::map<std::string, std::string> &params, PlanTarget &target)->void
{
	enable_moveEAC = false;
	target.option = aris::plan::Plan::NOT_RUN_EXECUTE_FUNCTION;

}
MoveStop::MoveStop(const std::string &name) :Plan(name)
{
	command().loadXmlStr(
		"<Command name=\"moveStop\">"
		"</Command>");
}




// 传感器信号测试 //
struct SensorParam
{
    int time;
    float ch1,ch2,ch3,ch4,ch5,ch6;
};
auto Sensor::prepairNrt(const std::map<std::string, std::string> &params, PlanTarget &target)->void
    {
        SensorParam param;
        for (auto &p : params)
        {
            if (p.first == "time")
            {
                param.time = std::stoi(p.second);
            }
        }

        target.param = param;
        //将所有option设置为NOT_CHECK_ENABLE
        std::fill(target.mot_options.begin(), target.mot_options.end(), Plan::NOT_CHECK_ENABLE);
    }
auto Sensor::executeRT(PlanTarget &target)->int
    {
        auto &param = std::any_cast<SensorParam&>(target.param);
        // 访问主站 //
        auto controller = dynamic_cast<aris::control::EthercatController*>(target.controller);

        int16_t rawData;
        double volToSig = 10.0;
        //主站下标为at(1),从站下标从at(2)开始
        //读取PDO，第一参数为index，第二参数为subindex，第三参数读取数据，第四参数为操作位数
        //16位精度
        controller->slavePool().at(2).readPdo(0x6000, 0x11, &rawData, 16);
        param.ch1 = volToSig *rawData/65535.0*522.5;
        controller->slavePool().at(2).readPdo(0x6010, 0x11, &rawData, 16);
        param.ch2 = volToSig*rawData/65535;
        controller->slavePool().at(2).readPdo(0x6020, 0x11, &rawData, 16);
        param.ch3 = volToSig*rawData/65535;
        controller->slavePool().at(3).readPdo(0x6000, 0x11, &rawData, 16);
        param.ch4 = volToSig *rawData/65535;
        controller->slavePool().at(3).readPdo(0x6010, 0x11, &rawData, 16);
        param.ch5 = volToSig*rawData/65535;
        controller->slavePool().at(3).readPdo(0x6020, 0x11, &rawData, 16);
        param.ch6 = volToSig*rawData/65535;



        //print//
        //setw(n)设置输出宽度为n
        auto &cout = controller->mout();
        if (target.count % 100 == 0)
        {
            cout << std::setw(6) << param.ch1 << "  ";
            //cout << std::setw(6) << param.ch2 << "  ";
            //cout << std::setw(6) << param.ch3 << "  ";
            //cout << std::setw(6) << param.ch4 << "  ";
            //cout << std::setw(6) << param.ch5 << "  ";
            //cout << std::setw(6) << param.ch6 << "  ";

            cout << std::endl;
            cout << "----------------------------------------------------" << std::endl;
        }

        //log//
        auto &lout = controller->lout();
        {
            lout << param.ch1 << " ";
            lout << param.ch2 << " ";
            lout << param.ch3 << " ";
            lout << param.ch4 << " ";
            lout << param.ch5 << " ";
            lout << param.ch6 << " ";

            lout << std::endl;
        }
        param.time--;
        return param.time;
    }
auto Sensor::collectNrt(PlanTarget &target)->void {}
Sensor::Sensor(const std::string &name) :Plan(name)
{
    command().loadXmlStr(
        "<Command name=\"sensor\">"
        "	<GroupParam>"
        "		<Param name=\"time\" default=\"100000\"/>"
        "	</GroupParam>"
        "</Command>");
}

// 将创建的轨迹添加到轨迹规划池planPool中 //
auto createPlanRootRokaeXB4()->std::unique_ptr<aris::plan::PlanRoot>
{
	std::unique_ptr<aris::plan::PlanRoot> plan_root(new aris::plan::PlanRoot);

	plan_root->planPool().add<aris::plan::Enable>();
	plan_root->planPool().add<aris::plan::Disable>();
	plan_root->planPool().add<aris::plan::Mode>();
	plan_root->planPool().add<aris::plan::Show>();
	plan_root->planPool().add<aris::plan::Recover>();
	auto &rs = plan_root->planPool().add<aris::plan::Reset>();
	rs.command().findParam("pos")->setDefaultValue("{0.01}");
	plan_root->planPool().add<MoveJS>();
	plan_root->planPool().add<MoveEAP>();
	plan_root->planPool().add<MoveEAC>();
	plan_root->planPool().add<MoveStop>();
    plan_root->planPool().add<Sensor>();
	return plan_root;
}


// 主函数
int main(int argc, char *argv[])
{
	//创建Ethercat主站对象
    //aris::control::EthercatMaster mst;
	//自动扫描，连接从站
    //mst.scan();
    //std::cout<<mst.xmlString()<<std::endl;

	//cs代表成员函数的引用，aris是头文件，server是命名空间，ControlServer是结构体
    auto&cs = aris::server::ControlServer::instance();
    cs.resetController(createControllerRokaeXB4().release());
    cs.resetPlanRoot(createPlanRootRokaeXB4().release());

    std::cout<<"start controller server"<<std::endl;
	//启动线程
    cs.start();
    std::cout<<"start controller server"<<std::endl;
	//getline是将输入的值赋值给command_in
	for (std::string command_in; std::getline(std::cin, command_in);)
	{
		try
		{
			auto id = cs.executeCmd(aris::core::Msg(command_in));
			std::cout << "command id:" << id << std::endl;
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << std::endl;
			LOG_ERROR << e.what() << std::endl;
		}
	}
}
