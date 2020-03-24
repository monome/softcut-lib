switch(id) {
    {%- for op in ops %}
    case  {{ op.name | cpp_enum }}:
        {%- if op.name is voice %}
        cut.voice(p->idx_0).{{op.name | cpp_voice_setter}}(/*FIXME*/);
        {%- else %}
        cut.{{op.name | cpp_setter}}(/*FIXME*/);
        {%- endif %}
        break;
    {%- endfor %}
} AudioCommand;