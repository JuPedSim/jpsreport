# Configuration file for the Sphinx documentation builder.

# -- Project information

project = 'JPSreport'
copyright = '2022, Forschungszentrum Juelich GmbH, IAS-7'
author = 'JuPedSim Development Team'

release = '0.9.0'
version = '0.9.0'

# -- General configuration

extensions = [
    'sphinx.ext.duration',
    'sphinx.ext.doctest',
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',
    'sphinx.ext.intersphinx',
    'sphinx.ext.autosectionlabel',
    'myst_parser',
]

intersphinx_mapping = {
    'python': ('https://docs.python.org/3/', None),
    'sphinx': ('https://www.sphinx-doc.org/en/master/', None),
}
intersphinx_disabled_domains = ['std']

templates_path = ['_templates']

# -- Options for HTML output

html_theme = 'sphinx_rtd_theme'

# -- Options for EPUB output
epub_show_urls = 'footnote'

html_static_path = ['_static']
html_logo = "images/jupedsim_small.png"

html_theme_options = {
    'navigation_depth': -1,
    'logo_only': True,
    'style_nav_header_background': 'white',
    'collapse_navigation': False,
    'titles_only': False
}
