scripts=imgconv.sh svg2png.sh docxproc.sh latexproc.sh Makefile
images=`ls image`
mermaid=`ls image/*.mermaid`
svgs=`ls image/*.mermaid|sed 's/.mermaid/.svg/g'`
pdfs=`ls image/*.mermaid|sed 's/.mermaid/.pdf/g'`
pngs=`ls image/*.mermaid|sed 's/.mermaid/.png/g'`

%.pdf:%.mermaid $(scripts)
	mmdc -f -i $< -o $@

%.svg:%.mermaid $(scripts)
	mmdc -f -i $< -o $@

%.png:%.mermaid $(scripts)
	mmdc -f -C image/css.css -c image/config.json --scale 8 -i $< -o $@


%.docx:%.md $(scripts)
	make $(svgs)
	make $(pngs)
	./docxproc.sh $<
	pandoc --reference-doc=template3.docx -s -t docx --filter pandoc-crossref --citeproc --bibliography=bib/references.bib --csl=bib/chinese-gb7714-2005-numeric.csl -o $@ $@.md
	rm -rvf $@.md

%.pdf:%.md eisvogel.latex $(scripts)
	make $(svgs)
	make $(pdfs)
	./latexproc.sh $<
	pandoc --filter pandoc-xnos --template eisvogel.latex --columns=50 --number-sections --listings --top-level-division="chapter" --bibliography=bib/references.bib --csl=bib/chinese-gb7714-2005-numeric.csl -t pdf --pdf-engine=xelatex -o $@ $@.md
	rm -rvf $@.md
