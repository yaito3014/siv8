# include <Siv3D.hpp>

void Main()
{
	Scene::SetBackground(ColorF{ 0.2, 0.5, 0.7 });

	const Font font{ 48, Typeface::Bold };

	while (System::Update())
	{
		font(U"Hello, Siv3D!").drawAt(Scene::Center(), ColorF{ 1.0 });
	}
}
