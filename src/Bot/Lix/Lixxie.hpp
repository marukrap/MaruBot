#pragma once

#include "Job.hpp"
#include "Map.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Time.hpp>

class World;

class Lixxie : public sf::Drawable, private sf::Transformable
{
public:
	using Ptr = std::unique_ptr<Lixxie>;

	enum Style
	{
		Garden,
		Highlight,
		Neutral,
		Red,
		Orange,
		Yellow,
		Green,
		Blue,
		Purple,
		Grey,
		Black,
		Max
	};

public:
	Lixxie(World& world, Style style, const std::string& id);

	Lixxie(const Lixxie&) = delete;
	Lixxie& operator=(const Lixxie&) = delete;

	const std::string& getId() const;

	void setText(const std::string& str, const sf::Font& font, unsigned int fontSize);
	void setText(const std::string& str);

	void setColor(const sf::Color& color);

	sf::FloatRect getBoundingRect() const;

	// LixD/lixxie.d
	const Map& getEnv() const;

	Style getStyle() const;

	int getEx() const;
	int getEy() const;

	void setEx(int x);
	void setEy(int y);

	Job& getJob();
	const Job& getJob() const;

	Activity getActivity() const;
	PhysicsUpdateOrder getUpdateOrder() const;

	World& getWorld();

	int getPloderTimer() const;
	void setPloderTimer(int timer);

	int getFlingX() const;
	int getFlingY() const;

	int getFrame() const;
	void setFrame(int frame);

	// xf, yf

	Map::Flags getBodyEncounters() const;
	Map::Flags getFootEncounters() const;

	bool getFacingRight() const;
	bool getMirror() const;
	int getRotation() const;
	int getDirection() const;
	void setDirection(int dir);

	void turn();

	//
	void setNoEncountersNoBlockerFlags();
	void forceBodyAndFootEncounters(Map::Flags body, Map::Flags foot);
	void addEncountersFromHere();

	void moveAhead(int plusX = 2);
	void moveDown(int plusY);
	void moveUp(int minusY);

	void addFling(int px, int py, bool sameTribe);
	void resetFlingNew();

	//
	bool isSolid(int px = 0, int py = 2) const;
	bool isSolidSingle(int px = 0, int py = 2) const;
	bool getSteel(int px, int py) const;
	bool wouldHitSteel(const Mask& mask) const;
	int solidWallHeight(int px = 0, int py = 0) const;
	int countSolid(int x1, int y1, int x2, int y2) const;
	int countSteel(int x1, int y1, int x2, int y2) const;

	void playSound(const std::string& id);

	bool isLastFrame() const;
	void advanceFrame();

	//
	bool healthy() const;
	bool cursorShouldOpenOverMe() const;

	int priorityForNewAc(Activity newAc) const;

	void become(Activity newAc);
	void assignManually(Activity newAc);
	void perform();
	void becomePloder();
	void applyFlingXY();

	static std::vector<std::unique_ptr<sf::Texture>> generateTextures();

private:
	void becomeTemplate(bool manualAssignment, Activity newAc);

	void updateTextureRect();

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

public:
	static constexpr int PloderDelay = 75;

private:
	static constexpr float SpriteOffsetX = 16.f;
	static constexpr float SpriteOffsetY = 26.f;

	World& m_world;
	std::string m_id; // username
	sf::Sprite m_sprite;
	sf::Text m_text;
	sf::Text m_textShadow;

	// LixD/lixxie.d
	std::vector<Job::Ptr> m_oldJobs; // TODO: remove
	Job::Ptr m_job = nullptr;
	int m_ex = 0;
	int m_ey = 0;
	int m_flingX = 0;
	int m_flingY = 0;
	int m_ploderTimer = 0;
	Style m_style;
	Map::Flags m_encBody = Map::None;
	Map::Flags m_encFoot = Map::None;

public:
	__declspec(property(get = getEnv)) Map& env;
	__declspec(property(get = getStyle)) Style style;
	__declspec(property(get = getEx, put = setEx)) int ex;
	__declspec(property(get = getEy, put = setEy)) int ey;
	__declspec(property(get = getJob)) Job& job;
	__declspec(property(get = getActivity)) Activity activity;
	__declspec(property(get = getUpdateOrder)) PhysicsUpdateOrder updateOrder;
	__declspec(property(get = getWorld)) World& world;
	__declspec(property(get = getPloderTimer, put = setPloderTimer)) int ploderTimer;
	__declspec(property(get = getFlingX)) int flingX;
	__declspec(property(get = getFlingY)) int flingY;
	__declspec(property(get = getFrame, put = setFrame)) int frame;
	__declspec(property(get = getBodyEncounters)) Map::Flags bodyEncounters;
	__declspec(property(get = getFootEncounters)) Map::Flags footEncounters;

	__declspec(property(get = getFacingRight)) bool facingRight;
	__declspec(property(get = getMirror)) bool mirror;
	__declspec(property(get = getRotation)) int rotation;
	__declspec(property(get = getDirection, put = setDirection)) int direction;

	// flags
	bool facingLeft          = false;
	bool abilityToRun        = false;
	bool abilityToClimb      = false;
	bool abilityToFloat      = false;
	bool marked              = false;
	bool ploderIsExploder    = false;
	bool flingNew            = false;
	bool flingBySameTribe    = false;
	bool turnedByBlocker     = false;
	bool inBlockerFieldLeft  = false;
	bool inBlockerFieldRight = false;
};
