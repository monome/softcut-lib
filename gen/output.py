from jinja2 import Environment, PackageLoader

import operations

default_output_location = "./output"

env = Environment(
    loader=PackageLoader('main', 'templates')
)

#---- custom filters and tests for jinja

# "foo bar" -> "FOO_BAR"
env.filters['cpp_enum'] = lambda str: str.replace(" ", "_").upper()

# "foo bar" -> "setFooBar"
env.filters['cpp_setter'] = lambda str: "set{}".format(str.title().replace(" ", ""))

# "voice foo bar" -> "setFooBar"
env.filters['cpp_voice_setter'] = lambda str: "set{}".format(str.replace("voice ", "").title().replace(" ", ""))

# "foo bar" -> "/softcut/foo/bar"
env.filters['osc_path'] = lambda str: "/softcut/{}".format(str.replace(" ", "/"))

# "voice foo bar" -> True, "foo bar" -> False
env.tests['voice'] = lambda str: "voice" in str

#--- functions
def write_file(text, name, location):
    path = '{}/{}'.format(location, name)
    try:
        with open(path, 'w') as pagefile:
            pagefile.write(text)
            pagefile.close()
            print("exported: {}".format(path))
    except IOError:
        print("error exporting page at location: {}".format(path))

def perform(filename, location=None):
    if location is None: location = default_output_location;
    template = env.get_template(filename)
    text = template.render(ops=operations.ops)
    write_file(text, filename, location)


# def generate_single_page():
#     template = env.get_template('index.html')
#     html = template.render(site_root='.', userdata=ScUserData.user_data)
#     export_page('index.html', html)

