#include "Job.hpp"

// skills
#include "Skill/Ascender.hpp"
#include "Skill/Basher.hpp"
#include "Skill/Batter.hpp"
#include "Skill/Blocker.hpp"
#include "Skill/Builder.hpp"
#include "Skill/Climber.hpp"
#include "Skill/Cuber.hpp"
#include "Skill/Digger.hpp"
#include "Skill/Exiter.hpp"
#include "Skill/Exploder.hpp"
#include "Skill/Faller.hpp"
#include "Skill/Floater.hpp"
#include "Skill/Lander.hpp"
#include "Skill/Miner.hpp"
#include "Skill/Stunner.hpp"
#include "Skill/Tumbler.hpp"
#include "Skill/Walker.hpp"

#include <cassert>

Job::Job(Lixxie& lixxie)
	: m_lixxie(lixxie)
{
}

Activity Job::getActivity() const
{
	return m_activity;
}

int Job::getFrame() const
{
	return m_frame;
}

void Job::setFrame(int frame)
{
	m_frame = frame;
}

int Job::getSpriteOffsetX() const
{
	return m_spriteOffsetX;
}

void Job::setSpriteOffsetX(int offsetX)
{
	m_spriteOffsetX = offsetX;
}

PhysicsUpdateOrder Job::updateOrder() const
{
	return PhysicsUpdateOrder::Peaceful;
}

bool Job::blockable() const
{
	return true;
}

AfterAssignment Job::onManualAssignment(Job& old)
{
	return AfterAssignment::BecomeNormally;
}

void Job::onBecome(const Job& old)
{
}

void Job::perform()
{
}

bool Job::healthy(Activity activity)
{
	return activity != Activity::Nothing && activity != Activity::Splatter && activity != Activity::Burner
		&& activity != Activity::Drowner && activity != Activity::Imploder && activity != Activity::Exploder
		&& activity != Activity::Exiter  && activity != Activity::Cuber;
}

Job::Ptr Job::createJob(Activity activity, Lixxie& lixxie)
{
	Job::Ptr job = nullptr;

	switch (activity)
	{
	case Activity::Nothing:    job = std::make_shared<RemovedLix>(lixxie); break;
	case Activity::Faller:     job = std::make_shared<Faller>(lixxie);     break;
	case Activity::Tumbler:    job = std::make_shared<Tumbler>(lixxie);    break;
	case Activity::Stunner:    job = std::make_shared<Stunner>(lixxie);    break;
	case Activity::Lander:     job = std::make_shared<Lander>(lixxie);     break;
	case Activity::Splatter:   job = std::make_shared<Splatter>(lixxie);   break;
	case Activity::Burner:     job = std::make_shared<Burner>(lixxie);     break;
	case Activity::Drowner:    job = std::make_shared<Drowner>(lixxie);    break;
	case Activity::Exiter:     job = std::make_shared<Exiter>(lixxie);     break;
	case Activity::Walker:     job = std::make_shared<Walker>(lixxie);     break;

	case Activity::Runner:     job = std::make_shared<Runner>(lixxie);     break;
	case Activity::Climber:    job = std::make_shared<Climber>(lixxie);    break;
	case Activity::Ascender:   job = std::make_shared<Ascender>(lixxie);   break;
	case Activity::Floater:    job = std::make_shared<Floater>(lixxie);    break;
	case Activity::Imploder:   job = std::make_shared<Imploder>(lixxie);   break;
	case Activity::Exploder:   job = std::make_shared<Exploder>(lixxie);   break;
	case Activity::Blocker:    job = std::make_shared<Blocker>(lixxie);    break;
	case Activity::Builder:    job = std::make_shared<Builder>(lixxie);    break;
	case Activity::Shrugger:   job = std::make_shared<Shrugger>(lixxie);   break;
	case Activity::Platformer: job = std::make_shared<Platformer>(lixxie); break;
	case Activity::Shrugger2:  job = std::make_shared<Shrugger>(lixxie);   break;
	case Activity::Basher:     job = std::make_shared<Basher>(lixxie);     break;
	case Activity::Miner:      job = std::make_shared<Miner>(lixxie);      break;
	case Activity::Digger:     job = std::make_shared<Digger>(lixxie);     break;

	case Activity::Jumper:     job = std::make_shared<Jumper>(lixxie);     break;
	case Activity::Batter:     job = std::make_shared<Batter>(lixxie);     break;
	case Activity::Cuber:      job = std::make_shared<Cuber>(lixxie);      break;
	}

	assert(job && job->activity == Activity::Nothing);
	job->m_activity = activity;

	return job;
}
