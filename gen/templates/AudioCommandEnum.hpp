typedef enum {
    {%- for op in ops %}
    // {{ op.desc_short }} ({{ op.format }})
    {{ op.name | replace(" ", "_") | upper }},
    {%- endfor %}
} AudioCommand;