# Security Policy

Creating a [security policy](https://imagemagick.org/script/security-policy.php) that fits your specific local environment before making use of ImageMagick is highly advised.

## Supported Versions

We encourage users to upgrade to the latest ImageMagick release to ensure that all known security vulnerabilities are addressed.  On request, we can backport security fixes to other ImageMagick versions.

## Reporting a Vulnerability

Before you post a vulnerability, first determine if the vulnerability can be mitigated by a properly curated security policy.  Next, verify your policy using the [validation tool](https://imagemagick-secevaluator.doyensec.com/).  Now use a [development container](https://containers.dev/), available in the `.devcontainer/security` folder, to verify that the security issue can be reproduced with the latest source code and your security policy.  If you feel confident that the security policy does not address the vulnerability, post the vulnerability as a [security advisory](https://github.com/ImageMagick/ImageMagick/security/advisories/new).  Most vulnerabilities are reviewed and resolved within 48 hours.
