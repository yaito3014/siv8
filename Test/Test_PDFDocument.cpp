//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2026 Ryo Suzuki
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include "Siv3DTest.hpp"
# include <array>
# include <format>
# include <string>
# include <vector>

// The engine loads PDFium at run time on Windows and macOS only.
# if SIV3D_PLATFORM(WINDOWS) || SIV3D_PLATFORM(MACOS)

namespace
{
	// A one-page PDF whose MediaBox is 200 x 100 points, with an exact xref table.
	[[nodiscard]]
	std::string MakeOnePagePDF()
	{
		constexpr std::array<std::string_view, 3> Objects
		{
			"<< /Type /Catalog /Pages 2 0 R >>",
			"<< /Type /Pages /Kids [3 0 R] /Count 1 >>",
			"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 200 100] >>",
		};

		std::string pdf = "%PDF-1.4\n";
		std::vector<size_t> offsets;

		for (size_t i = 0; i < Objects.size(); ++i)
		{
			offsets.push_back(pdf.size());
			pdf += std::format("{} 0 obj\n{}\nendobj\n", (i + 1), Objects[i]);
		}

		const size_t xrefOffset = pdf.size();
		pdf += std::format("xref\n0 {}\n0000000000 65535 f \n", (Objects.size() + 1));

		for (const size_t offset : offsets)
		{
			pdf += std::format("{:010} 00000 n \n", offset);
		}

		pdf += std::format("trailer\n<< /Size {} /Root 1 0 R >>\nstartxref\n{}\n%%EOF\n", (Objects.size() + 1), xrefOffset);
		return pdf;
	}
}

TEST_CASE("PDFDocument.loadAndRender")
{
	const std::string source = MakeOnePagePDF();
	const PDFDocument document{ MemoryViewReader{ source.data(), source.size() } };
	REQUIRE(document);

	CHECK(document.pageCount() == 1);
	CHECK(document.getPageSize(0) == Size{ 200, 100 });
	CHECK(document.getPageSize(0, 2.0) == Size{ 400, 200 });

	const PDFPage page = document.renderPage(0, PDFRenderOptions{ .scale = 2.0, .backgroundColor = Color{ 255, 0, 0 } });
	CHECK(page.image.size() == Size{ 400, 200 });
	CHECK(page.image[0][0] == Color{ 255, 0, 0 });
	CHECK(page.image[199][399] == Color{ 255, 0, 0 });
}

# endif
