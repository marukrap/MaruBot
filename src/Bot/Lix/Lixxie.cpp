#include "Lixxie.hpp"
#include "World.hpp"
#include "Help.hpp"
#include "Skill/Walker.hpp"
#include "Skill/Tumbler.hpp"
#include "../../Utility.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <cassert>

namespace
{
	std::vector<sf::Texture*> Textures;
	std::vector<bool> ExistingFrames;
	sf::Vector2i FrameSize;
	sf::Vector2i NumFrames;
}

Lixxie::Lixxie(World& world, Style style, const std::string& id)
	: m_world(world)
	, m_id(id)
	, m_sprite(*Textures[style])
	, m_style(style)
{
	m_job = Job::createJob(Activity::Faller, *this);
	frame = 4;
	addEncountersFromHere();

	updateTextureRect();

	abilityToClimb = true;
	abilityToFloat = true;
}

const std::string& Lixxie::getId() const
{
	return m_id;
}

void Lixxie::setText(const std::string& str, const sf::Font& font, unsigned int fontSize)
{
	m_text.setFont(font);
	m_text.setCharacterSize(fontSize);
	m_text.setPosition(0.f, -static_cast<float>(FrameSize.y));

	m_textShadow.setFont(font);
	m_textShadow.setCharacterSize(fontSize);
	m_textShadow.setPosition(m_text.getPosition() + sf::Vector2f(1.f, 1.f));
	m_textShadow.setFillColor(sf::Color::Black);

	setText(str);
}

void Lixxie::setText(const std::string& str)
{
	m_text.setString(str);
	centerOrigin(m_text);

	m_textShadow.setString(str);
	m_textShadow.setOrigin(m_text.getOrigin());
}

void Lixxie::setColor(const sf::Color& color)
{
	m_text.setFillColor(color);
}

sf::FloatRect Lixxie::getBoundingRect() const
{
	return getTransform().transformRect(m_sprite.getGlobalBounds());
}

const Map& Lixxie::getEnv() const
{
	return m_world.lookup;
}

Lixxie::Style Lixxie::getStyle() const
{
	return m_style;
}

int Lixxie::getEx() const
{
	return m_ex;
}

int Lixxie::getEy() const
{
	return m_ey;
}

void Lixxie::setEx(int x)
{
	m_ex = even(x);

	if (env.torusX)
		m_ex = positiveMod(m_ex, env.width);

	addEncountersFromHere();
	setPosition(static_cast<float>(m_ex), static_cast<float>(m_ey));
}

void Lixxie::setEy(int y)
{
	m_ey = y;

	if (env.torusY)
		m_ey = positiveMod(m_ey, env.height);

	addEncountersFromHere();
	setPosition(static_cast<float>(m_ex), static_cast<float>(m_ey));
}

Job& Lixxie::getJob()
{
	return *m_job;
}

const Job& Lixxie::getJob() const
{
	return *m_job;
}

Activity Lixxie::getActivity() const
{
	return m_job->activity;
}

PhysicsUpdateOrder Lixxie::getUpdateOrder() const
{
	return m_job->updateOrder();
}

World& Lixxie::getWorld()
{
	return m_world;
}

int Lixxie::getPloderTimer() const
{
	return m_ploderTimer;
}

void Lixxie::setPloderTimer(int timer)
{
	m_ploderTimer = timer;
}

int Lixxie::getFlingX() const
{
	return m_flingX;
}

int Lixxie::getFlingY() const
{
	return m_flingY;
}

int Lixxie::getFrame() const
{
	return m_job->frame;
}

void Lixxie::setFrame(int frame)
{
	m_job->frame = frame;
}

Map::Flags Lixxie::getBodyEncounters() const
{
	return m_encBody;
}

Map::Flags Lixxie::getFootEncounters() const
{
	return m_encFoot;
}

bool Lixxie::getFacingRight() const
{
	return !facingLeft;
}

bool Lixxie::getMirror() const
{
	return facingLeft;
}

int Lixxie::getRotation() const
{
	return 2 * facingLeft;
}

int Lixxie::getDirection() const
{
	return facingLeft ? - 1 : 1;
}

void Lixxie::setDirection(int dir)
{
	assert(dir != 0);
	facingLeft = (dir < 0);
}

void Lixxie::turn()
{
	facingLeft = !facingLeft;
}

void Lixxie::setNoEncountersNoBlockerFlags()
{
	m_encBody           = Map::None;
	m_encFoot           = Map::None;
	inBlockerFieldLeft  = false;
	inBlockerFieldRight = false;
	turnedByBlocker     = false;
}

void Lixxie::forceBodyAndFootEncounters(Map::Flags body, Map::Flags foot)
{
	m_encBody = body;
	m_encFoot = foot;
}

void Lixxie::addEncountersFromHere()
{
	m_encFoot |= m_world.lookup.get({ m_ex, m_ey });
	m_encBody |= m_encFoot
			  |  m_world.lookup.get({ m_ex, m_ey -  4 })
			  |  m_world.lookup.get({ m_ex, m_ey -  8 })
			  |  m_world.lookup.get({ m_ex, m_ey - 12 });
}

void Lixxie::moveAhead(int plusX)
{
	if (inBlockerFieldLeft && inBlockerFieldRight)
		return;

	plusX = even(plusX) * direction;

	for (; plusX > 0; plusX -= 2)
		ex = m_ex + 2;
	for (; plusX < 0; plusX += 2)
		ex = m_ex - 2;
}

void Lixxie::moveDown(int plusY)
{
	for (; plusY > 0; --plusY)
		ey = m_ey + 1;
	for (; plusY < 0; ++plusY)
		ey = m_ey - 1;
}

void Lixxie::moveUp(int minusY)
{
	moveDown(-minusY);
}

void Lixxie::addFling(int px, int py, bool sameTribe)
{
	if (flingBySameTribe && sameTribe)
		return;

	flingBySameTribe = (flingBySameTribe || sameTribe);
	flingNew         = true;
	m_flingX += px;
	m_flingY += py;
}

void Lixxie::resetFlingNew()
{
	flingNew         = false;
	flingBySameTribe = false;
	m_flingX         = 0;
	m_flingY         = 0;
}

bool Lixxie::isSolid(int px, int py) const
{
	return m_world.lookup.getSolidEven({ m_ex + px * direction, m_ey + py });
}

bool Lixxie::isSolidSingle(int px, int py) const
{
	return m_world.lookup.getSolid({ m_ex + px * direction, m_ey + py });
}
bool Lixxie::getSteel(int px, int py) const
{
	return m_world.lookup.getSteel({ m_ex + px * direction, m_ey + py });
}

bool Lixxie::wouldHitSteel(const Mask& mask) const
{
	return m_world.lookup.getSteelUnlessMaskIgnores({ m_ex, m_ey }, mask);
}

int Lixxie::solidWallHeight(int px, int py) const
{
	int solid = 0;

	for (int i = 1; i > -Walker::HighestStepUp; --i)
	{
		if (isSolid(px, py + i))
			++solid;
		else
			break;
	}

	return solid;
}

int Lixxie::countSolid(int x1, int y1, int x2, int y2) const
{
	if (x2 < x1)
		std::swap(x1, x2);
	if (y2 < y1)
		std::swap(y1, y2);

	int ret = 0;

	for (int ix = even(x1); ix <= even(x2); ix += 2)
		for (int iy = y1; iy <= y2; ++iy)
		{
			if (isSolid(ix, iy))
				++ret;
		}

	return ret;
}

int Lixxie::countSteel(int x1, int y1, int x2, int y2) const
{
	if (x2 < x1)
		std::swap(x1, x2);
	if (y2 < y1)
		std::swap(y1, y2);

	int ret = 0;

	for (int ix = even(x1); ix <= even(x2); ix += 2)
		for (int iy = y1; iy <= y2; ++iy)
		{
			if (getSteel(ix, iy))
				++ret;
		}

	return ret;
}

void Lixxie::playSound(const std::string& id)
{
	m_world.playSound(id);
}

bool Lixxie::isLastFrame() const
{
	const int tu = frame + 1;
	const int tv = static_cast<int>(activity);

	if (tu < 0 || tu >= NumFrames.x || tv < 0 || tv >= NumFrames.y)
		return true;

	return !ExistingFrames[tu + tv * NumFrames.x];
}

void Lixxie::advanceFrame()
{
	frame = isLastFrame() ? 0 : frame + 1;
}

bool Lixxie::healthy() const
{
	return Job::healthy(activity);
}

bool Lixxie::cursorShouldOpenOverMe() const
{
	return healthy();
}

int Lixxie::priorityForNewAc(Activity newAc) const
{
	if (!cursorShouldOpenOverMe())
		return 0;

	if ((newAc == Activity::Imploder && m_ploderTimer > 0) ||
		(newAc == Activity::Exploder && m_ploderTimer > 0))
		// LIXBOT: turn off abilities
		// (newAc == Activity::Runner   && abilityToRun) ||
		// (newAc == Activity::Climber  && abilityToClimb) ||
		// (newAc == Activity::Floater  && abilityToFloat))
		return 1;

	constexpr bool avoidBuilderQueuing = true;
	constexpr bool avoidBatterToExploder = false;

	int priority = 0;

	switch (activity)
	{
	case Activity::Blocker:
		if (newAc == Activity::Walker ||
			newAc == Activity::Blocker || // LIXBOT: turn off blocker
			newAc == Activity::Imploder ||
			newAc == Activity::Exploder)
			priority = 6000;
		else
			return 1;
		break;

	case Activity::Stunner:
		if (frame >= 16)
		{
			priority = 3000;
			break;
		}
		else
			goto GOTO_TARGET_FULL_ATTENTION;

	case Activity::Ascender:
		if (frame >= 5)
		{
			priority = 3000;
			break;
		}
		else
			goto GOTO_TARGET_FULL_ATTENTION;

	case Activity::Faller:
	case Activity::Tumbler:
	case Activity::Climber:
	case Activity::Floater:
	case Activity::Jumper:
	GOTO_TARGET_FULL_ATTENTION:
		if (newAc == Activity::Runner ||
			newAc == Activity::Climber ||
			newAc == Activity::Floater ||
			newAc == Activity::Imploder ||
			newAc == Activity::Exploder)
			priority = 2000;
		// LIXBOT: allow jump while climbing
		else if (activity == Activity::Climber
			&& newAc == Activity::Jumper)
			priority = 2000;
		//
		else
			return 1;
		break;

	case Activity::Walker:
	case Activity::Lander:
	case Activity::Runner:
		priority = 3000;
		break;

	case Activity::Builder:
	case Activity::Platformer:
		if (newAc == activity)
			priority = avoidBuilderQueuing ? 1000 : 4000;
		else
			priority = 5000;
		break;

	default:
		if (newAc != activity)
			priority = 5000;
		else
			return 1;
	}

	priority += (newAc == Activity::Batter && avoidBatterToExploder ? -m_ploderTimer : m_ploderTimer);
	priority += 400 * abilityToRun + 200 * abilityToClimb + 100 * abilityToFloat;

	return priority;
}

void Lixxie::become(Activity newAc)
{
	becomeTemplate(false, newAc);
}

void Lixxie::assignManually(Activity newAc)
{
	becomeTemplate(true, newAc);
}

void Lixxie::perform()
{
	// performUseGadgets(*this)
	{
		addEncountersFromHere();
		m_job->perform();
		/*
		killOutOfBounds();
		useWater();
		useNonconstantTraps();
		useFlingers();

		assert(outsideWorld);
		if (!outsideWorld.state.nuking)
			useGoals();
		*/
	}

	// HACK: remove old jobs
	if (!m_oldJobs.empty())
		m_oldJobs.clear();

	updateTextureRect();
}

void Lixxie::becomePloder()
{
	become(ploderIsExploder ? Activity::Exploder : Activity::Imploder);
}

void Lixxie::applyFlingXY()
{
	if (!healthy())
		return;

	Tumbler::applyFlingXY(*this);
}

std::vector<std::unique_ptr<sf::Texture>> Lixxie::generateTextures()
{
	// LixD/cutbit.d
	sf::Image image;
	image.loadFromFile("Lix/Images/lix.I.png");

	const int width = image.getSize().x;
	const int height = image.getSize().y;

	const sf::Color frameColor = image.getPixel(0, 0);

	if (width > 1 && height > 1 &&
		image.getPixel(0, 1) == frameColor &&
		image.getPixel(1, 0) == frameColor &&
		image.getPixel(1, 1) != frameColor)
	{
		int xl = 2;
		int yl = 2;

		for (; xl < width; ++xl)
		{
			if (image.getPixel(xl, 1) == frameColor)
			{
				--xl;
				break;
			}
		}

		for (; yl < height; ++yl)
		{
			if (image.getPixel(1, yl) == frameColor)
			{
				--yl;
				break;
			}
		}

		int xfs = 1; // number of x-frames existing
		int yfs = 1; // number of y-frames existing

		if (xl * 2 > width && yl * 2 > height)
		{
			xfs = width;
			yfs = height;
		}

		else
		{
			xfs = width / (xl + 1);
			yfs = height / (yl + 1);
		}

		ExistingFrames.resize(xfs * yfs);

		if (xfs == 1 && yfs == 1)
			ExistingFrames[0] = true;

		else
		{
			for (int y = 0; y < yfs; ++y)
				for (int x = 0; x < xfs; ++x)
				{
					const int xf = x * (xl + 1) + 1;
					const int yf = y * (yl + 1) + 1;
					const bool hasFrameColor = (image.getPixel(xf, yf) == frameColor);
					ExistingFrames[x + y * xfs] = !hasFrameColor;
				}
		}

		FrameSize = { xl, yl };
		NumFrames = { xfs, yfs };
	}

	#if 0
	// HACK: erase background color and draw outlines
	// const std::vector<sf::Vector2i> cardinal = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
	const sf::Color bgColor = image.getPixel(1, 1);
	const sf::Color outlineColor = { 0, 0, 0, 128 };

	for (int y = 1; y < height - 1; ++y)
		for (int x = 1; x < width - 1; ++x)
		{
			if (image.getPixel(x, y) != bgColor)
				continue;

			const sf::Color color = image.getPixel(x - 1, y - 1);

			if (color != frameColor && color != bgColor & color != outlineColor && color != sf::Color::Transparent)
				image.setPixel(x, y, outlineColor);
			else
				image.setPixel(x, y, sf::Color::Transparent);

			/*
			bool eraseColor = true;

			for (const auto& dir : cardinal)
			{
				const sf::Color color = image.getPixel(x + dir.x, y + dir.y);

				if (color != frameColor && color != bgColor && color != sf::Color::Transparent)
				{
					eraseColor = false;
					break;
				}
			}

			if (eraseColor)
				image.setPixel(x, y, sf::Color::Transparent);
			*/
		}
	#endif

	// LixD/recol.d
	sf::Image recol;
	recol.loadFromFile("Lix/Images/lixrecol.I.png");

	const int recolHeight = recol.getSize().y; // number of styles
	const int recolWidth = recol.getSize().x; // number of colors

	assert(Style::Max + 1 == recolHeight);

	std::vector<std::vector<sf::Color>> recolArray(recolHeight);

	for (int y = 0; y < recolHeight; ++y)
	{
		recolArray[y].resize(recolWidth);

		for (int x = 0; x < recolWidth; ++x)
			recolArray[y][x] = recol.getPixel(x, y);
	}

	std::vector<std::unique_ptr<sf::Texture>> result(Style::Max);
	Textures.resize(Style::Max);

	for (int i = 0; i < Style::Max; ++i)
	{
		sf::Image img(image);

		for (int j = 0; j < recolWidth; ++j)
		{
			const sf::Color& from = recolArray[0][j]; // original
			const sf::Color& to = recolArray[i + 1][j];

			sf::Uint8* ptr = const_cast<sf::Uint8*>(img.getPixelsPtr());
			const sf::Uint8* end = ptr + (width * height * 4);

			while (ptr < end)
			{
				if ((ptr[0] == from.r) && (ptr[1] == from.g) && (ptr[2] == from.b) && (ptr[3] == from.a))
				{
					ptr[0] = to.r;
					ptr[1] = to.g;
					ptr[2] = to.b;
					ptr[3] = to.a;
				}

				ptr += 4;
			}
		}

		auto texture = std::make_unique<sf::Texture>();
		texture->loadFromImage(img);
		Textures[i] = texture.get();
		result[i] = std::move(texture);
	}

	return result;
}

void Lixxie::becomeTemplate(bool manualAssignment, Activity newAc)
{
	// Job::Ptr oldJob = m_job;
	m_oldJobs.emplace_back(m_job);
	m_job = Job::createJob(newAc, *this);

	const AfterAssignment afas = (manualAssignment ? m_job->onManualAssignment(*m_oldJobs.back()) : AfterAssignment::BecomeNormally);
	const bool restoreOldJob = (afas == AfterAssignment::DoNotBecome);

	// if (oldJobs.back()->activity != activity && !restoreOldJob)
		// oldJobs.back()->returnSkillsDontCallLixxieInHere(outsideWorld.tribe);

	if (restoreOldJob)
	{
		m_job = m_oldJobs.back();
		m_oldJobs.pop_back();
	}

	else if (afas == AfterAssignment::BecomeNormally)
	{
		m_job->onBecome(*m_oldJobs.back());

		if (manualAssignment)
			frame = frame - 1;
	}
}

void Lixxie::updateTextureRect()
{
	const int tu = frame;
	const int tv = static_cast<int>(activity);

	if (facingRight)
		m_sprite.setTextureRect({ tu * (FrameSize.x + 1) + 1, tv * (FrameSize.y + 1) + 1, FrameSize.x, FrameSize.y });
	else
		m_sprite.setTextureRect({ (tu + 1) * (FrameSize.x + 1), tv * (FrameSize.y + 1) + 1, -FrameSize.x, FrameSize.y });

	m_sprite.setOrigin(16.f - m_job->spriteOffsetX, 26.f);
}

void Lixxie::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	target.draw(m_sprite, states);
	target.draw(m_textShadow, states);
	target.draw(m_text, states);
}
